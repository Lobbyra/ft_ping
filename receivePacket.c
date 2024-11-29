#include "./ft_ping.h"

#define BUF_SIZE 1024

static void printSuccResponse(
    struct sockaddr_in* rAddr,
    struct iphdr* ipHdr,
    struct s_ping* response
) {
    struct timeval now;

    gettimeofday(&now, NULL);
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

int receivePacket(const int sockfd, const pid_t pid, const uint16_t seqId) {
    char buf[BUF_SIZE];
    int recvStatus;
    struct sockaddr_in rAddr;
    socklen_t rAddrLen = sizeof(rAddr);
    struct s_ping *response;
    struct iphdr* ipHdr;

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
    if (
        calculateChecksum((void*)response, sizeof(*response)) !=
        response->header.checksum
    ) {
        fprintf(stderr, "printf: Received packet checksum failed\n");
    }
    if (response->header.type == 0) {
        printSuccResponse(&rAddr, ipHdr, response);
    } else {
        printErrResponse((char*)buf);
    }
    (void)pid;
    (void)seqId;
    return (1);
}
