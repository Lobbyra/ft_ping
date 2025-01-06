#include "./ft_ping.h"

#define BUF_SIZE 1024

static void printSuccResponse(
    struct timeval now,
    struct sockaddr_in* rAddr,
    struct iphdr* ipHdr,
    struct s_ping* response
) {
    printf(
        "%ld bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
        sizeof(*response),
        inet_ntoa(rAddr->sin_addr),
        ntohs(response->header.un.echo.sequence),
        ipHdr->ttl,
        getMsDiff(&response->sentTimestamp, &now)
    );
    fflush(stdout);
}

static void printErrResponse(char *respBuf) {
    (void)respBuf;
    printf("ERROR\n");
}

static void saveStats(
    struct timeval now,
    struct s_ping* response,
    struct PingStats* pingStats
) {
    double diff = getMsDiff(&response->sentTimestamp, &now);

    pingStats->receivedPaquet += 1;
    pingStats->total += diff;
    pingStats->totalsq += diff * diff;
    pingStats->avg = pingStats->total / pingStats->receivedPaquet;
    if (pingStats->min == 0 || pingStats->min > diff) {
        pingStats->min = diff;
    }
    if (pingStats->max == 0 || pingStats->max < diff) {
        pingStats->max = diff;
    }
    pingStats->stddev = (
        pingStats->totalsq / (double)pingStats->receivedPaquet -
        pingStats->avg * pingStats->avg
    );
}

int receivePacket(
    const int sockfd,
    const pid_t pid,
    const uint16_t seqId,
    struct PingStats *pingStats
) {
    char buf[BUF_SIZE];
    int recvStatus;
    struct sockaddr_in rAddr;
    socklen_t rAddrLen = sizeof(rAddr);
    struct s_ping *response;
    struct iphdr* ipHdr;
    struct timeval now;


    memset(buf, 0, sizeof(buf));
    recvStatus = recvfrom(
        sockfd,
        buf,
        sizeof(buf),
        0,
        (struct sockaddr*)&rAddr,
        &rAddrLen
    );
    if (recvStatus == -1) {
        if (errno != EINTR) {
            fprintf(stderr, "printf(%d): %s\n", recvStatus, strerror(errno));
        }
        return (-1);
    }
    ipHdr = (struct iphdr*)buf;
    response = (struct s_ping*) (buf + (ipHdr->ihl << 2));
    // if (
    //     calculateChecksum((void*)response, sizeof(*response)) !=
    //     response->header.checksum
    // ) {
    //     fprintf(stderr, "printf: Received packet checksum failed\n");
    // }
    if (response->header.type == 0) {
        gettimeofday(&now, NULL);
        printSuccResponse(now, &rAddr, ipHdr, response);
        saveStats(now, response, pingStats);
    } else {
        printErrResponse((char*)buf);
    }
    (void)pid;
    (void)seqId;
    return (1);
}
