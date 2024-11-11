#include "./ft_ping.h"

struct pingStat {
    double min;
    double avg;
    double max;
    double stddev;
    double total;
    double totalsq;
};

static volatile int keepRunning = 1;

void intHandler(int dummy) {
    (void)dummy;
    keepRunning = 0;
}

int openSocket() {
    int sock;
    int ttl = 201;

    sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock == -1) {
        return (sock);
    }
    if (setsockopt(sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) != 0) {
        // printf("\nSetting socket options to TTeL failed!\n");
        return (1);
    } else {
        // printf("\nSocket set to TTL...\n");
    }
    return (sock);
}

int receive(int sock, u_int16_t seqId) {
    int responseLen;
    char rbuf[1024];
    struct sockaddr_in rAddr;
    socklen_t rAddrLen = sizeof(rAddr);
    struct iphdr *ip_header;
    int iphdrlen;
    struct icmphdr *icmp_response;
    struct icmphdr *icmp_req = NULL;

    memset(rbuf, 0, sizeof(rbuf));
    responseLen = recvfrom(
        sock,
        rbuf,
        sizeof(rbuf),
        0,
        (struct sockaddr*)&rAddr,
        &rAddrLen
    );
    if (responseLen <= 0) {
        perror("No response received or error");
        return (1);
    } else {
        // Parse the IP and ICMP headers
        ip_header = (struct iphdr*) rbuf;
        iphdrlen = ip_header->ihl << 2;
        icmp_response = (struct icmphdr*) (rbuf + iphdrlen);

        printf(
            "%d bytes from %s: icmp_seq=%d ttl=%d ",
            responseLen,
            inet_ntoa(rAddr.sin_addr),
            seqId,
            ip_header->ttl
        );
        // Check if it's an ICMP Echo Reply
        if (icmp_response->type != ICMP_ECHOREPLY) {
            printf("❌❌❌❌❌❌❌");
            print_icmp_code(icmp_response->type, icmp_response->code, "");
            icmp_req = (struct icmphdr*)(rbuf + 8 + iphdrlen * 2);
            printf("REQ: \n");
            debugIcmp(icmp_req);
            printf("\n");
        }
    }
    return (0);
}

void saveStat(
    struct timeval* beforeTv,
    struct timeval* afterTv,
    struct pingStat *pStat,
    u_int16_t seqId
) {
    double beforeMicroSec = (
        ((double)beforeTv->tv_sec * 1000 + beforeTv->tv_usec) / 1000
    );
    double afterMicroSec = (
        ((double)afterTv->tv_sec * 1000 + afterTv->tv_usec) / 1000
    );
    double diffMsSec = (afterMicroSec - beforeMicroSec);
    pStat->total += diffMsSec;
    pStat->totalsq += diffMsSec*diffMsSec;
    if (seqId == 0) {
        pStat->avg = pStat->total;
    } else {
        pStat->avg = pStat->total / (double)seqId;
    }
    if (pStat->min == 0 || pStat->min > diffMsSec) {
        pStat->min = diffMsSec;
    }
    if (pStat->max == 0 || pStat->max < diffMsSec) {
        pStat->max = diffMsSec;
    }
    printf("time=%.3f ms\n", diffMsSec);
    pStat->stddev = pStat->totalsq / (double)seqId - pStat->avg * pStat->avg;
}

void printPingStats(struct pingStat *pStat) {
    printf ("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
	      pStat->min, pStat->avg, pStat->max, nsqrt (pStat->stddev, 0.0005));
}

int ft_ping(bool isVerbose, char* host) {
    struct addrinfo* destInfo;
    struct icmphdr* packet;
    struct sockaddr_in destAddr;
    int sock;
    struct timeval beforeTv;
    struct timeval afterTv;
    struct pingStat pStat;
    u_int16_t seqId;

    seqId = 0;
    pStat.min = 0;
    pStat.avg = 0;
    pStat.max = 0;
    pStat.stddev = 0;
    pStat.total = 0;
    signal(SIGINT, intHandler);
    printf("isVerbose %d, Host: %s\n", isVerbose, host);
    // Try a DNS resolution
    destInfo = getDest(host);
    if (destInfo == NULL) {
        return (1);
    }
    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family = AF_INET;
    destAddr.sin_addr = ((struct sockaddr_in*)destInfo->ai_addr)->sin_addr;
    sock = openSocket();
    if (sock == -1) {
        perror("Socket could not be created");
        freeaddrinfo(destInfo);
        return (1);
    }
    packet = craftIcmpPackage(seqId);
    printf(
        "PING %s (%s): %ld data bytes\n",
        host,
        inet_ntoa(destAddr.sin_addr),
        sizeof(struct icmphdr)
    );
    while (keepRunning) {
        fflush(stdout);
        gettimeofday(&beforeTv, NULL);
        if (
            sendto(
                sock,
                packet,
                sizeof(struct icmphdr),
                0,
                (struct sockaddr*)&destAddr,
                sizeof(destAddr)
            ) <= 0
        ) {
            perror("ft_ping: ");
        } else {
            if (receive(sock, seqId) == 0) {
                gettimeofday(&afterTv, NULL);
                saveStat(&beforeTv, &afterTv, &pStat, seqId + 1);
            }
        }
        sleep(1);
        free(packet);
        seqId++;
        packet = craftIcmpPackage(seqId);
    }
    printPingStats(&pStat);
    freeaddrinfo(destInfo);
    close(sock);
    return (0);
}
