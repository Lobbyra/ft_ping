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

static bool isChecksumCorrect(struct s_ping *response, bool onlyHeader) {
    unsigned short checksum;
    const unsigned checksumSave = response->header.checksum;
    size_t sumSize;

    if (onlyHeader == true) {
        sumSize = sizeof(struct icmphdr);
    } else {
        sumSize = sizeof(struct s_ping);
    }
    // The ping struct checksum must be computed 0 as checksum
    response->header.checksum = 0;
    checksum = (
        calculateChecksum((unsigned char*)&(*response), sumSize)
    );
    response->header.checksum = checksumSave;
    if (checksum != checksumSave) {
        fprintf(
            stderr,
            "printf: Received packet checksum failed "
        );
        fprintf(
            stderr,
            "(received: %x, computed: %x)\n",
            checksumSave,
            checksum
        );
    }
    return (checksum == checksumSave);
}

int receivePacket(
    const bool isVerbose,
    const int sockfd,
    const pid_t pid,
    struct PingStats *pingStats
) {
    char buf[BUF_SIZE];
    int recvStatus;
    struct sockaddr_in rAddr;
    socklen_t rAddrLen = sizeof(rAddr);
    struct s_ping *response;
    struct iphdr* ipHdr;
    struct timeval now;
    pid_t receivedPid;

    memset(buf, 0, sizeof(buf));
    // READ THE PING SOCKET
    recvStatus = recvfrom(
        sockfd,
        buf,
        sizeof(buf),
        0,
        (struct sockaddr*)&rAddr,
        &rAddrLen
    );

    // READING FAIL MANAGEMENT
    if (recvStatus == -1) {
        if (errno != EINTR) {
            fprintf(stderr, "printf(%d): %s\n", recvStatus, strerror(errno));
        }
        return (-1);
    }
    ipHdr = (struct iphdr*)buf;
    response = (struct s_ping*) (buf + (ipHdr->ihl * 4));
    receivedPid = htons(response->header.un.echo.id);
    // PROCESSING THE RESPONSE
    if (
        response->header.type == 0 &&
        receivedPid == pid &&
        isChecksumCorrect(response, false) == true
    ) {
        gettimeofday(&now, NULL);
        printSuccResponse(now, &rAddr, ipHdr, response);
        saveStats(now, response, pingStats);
    } else if (
        (receivedPid == pid || receivedPid == 0) &&
        isChecksumCorrect(response, true) == true
    ) {
        printErrResponse(pid, isVerbose, &rAddr, buf, recvStatus);
    }
    return (1);
}
