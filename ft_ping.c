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

int receive(int sock) {
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
            "%s %d bytes  from %s\n",
            icmp_response->type == ICMP_ECHOREPLY ? "✅" : "❌",
            responseLen,
            inet_ntoa(rAddr.sin_addr)
        );
        // Check if it's an ICMP Echo Reply
        if (icmp_response->type != ICMP_ECHOREPLY) {
            print_icmp_code(icmp_response->type, icmp_response->code, "");
            icmp_req = (struct icmphdr*)(rbuf + 8 + iphdrlen * 2);
            printf("REQ: \n");
            debugIcmp(icmp_req);
            printf("\n");
        }
        printf("RESP: \n");
        debugIcmp(icmp_response);
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
        beforeTv->tv_sec * 1000000 + beforeTv->tv_usec
    );
    double afterMicroSec = (
        afterTv->tv_sec * 1000000 + afterTv->tv_usec
    );
    double diffMsSec = (afterMicroSec - beforeMicroSec);
    printf("After: [%f], before: [%f], diff : %.3f\n", afterMicroSec, beforeMicroSec, diffMsSec);
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
    printf("stddev[%f]\n", pStat->stddev);
    pStat->stddev = pStat->totalsq / pStat->total - pStat->avg * pStat->avg;
}

void printPingStats(struct pingStat *pStat) {
    printf ("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
	      pStat->min / 1000, pStat->avg / 1000, pStat->max / 1000, nsqrt (pStat->stddev, 0.0005));
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
    while (keepRunning) {
        packet = craftIcmpPackage(seqId);
        printf(
            "seqId = %d, Pid = %d, Checksum = %d\n",
            ntohs(packet->un.echo.sequence),
            ntohs(packet->un.echo.id),
            packet->checksum
        );
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
            if (receive(sock) == 0) {
                gettimeofday(&afterTv, NULL);
                saveStat(&beforeTv, &afterTv, &pStat, seqId + 1);
            }
        }
        sleep(1);
        free(packet);
        seqId++;
    }
    printPingStats(&pStat);
    freeaddrinfo(destInfo);
    close(sock);
    return (0);
}
