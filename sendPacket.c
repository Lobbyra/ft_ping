#include "./ft_ping.h"

// Calculate the checksum (RFC 1071)
static unsigned short calculateChecksum(void *packet, int len) {
    unsigned short *buf = packet;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char*)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

static struct s_ping craftIcmpPackage(
    const pid_t pid,
    const size_t seqId
) {
    struct s_ping ping;

    ping.header.type = ICMP_ECHO;
    ping.header.code = 0;
    ping.header.un.echo.id = htons(pid);
    ping.header.un.echo.sequence = htons(seqId);
    gettimeofday(&(ping.sentTimestamp), NULL);
    ping.header.checksum = 0;
    ping.header.checksum = calculateChecksum(&ping, sizeof(struct s_ping));
    return (ping);
}

int sendPacket(
    const struct s_destInfo* destInfo,
    const pid_t pid,
    const size_t seqId
) {
    struct s_ping ping;
    int sendRet;

    ping = craftIcmpPackage(pid, seqId);
    sendRet = sendto(
        destInfo->sockfd,
        &ping,
        sizeof(ping),
        0,
        (struct sockaddr*)&(destInfo->addr),
        sizeof(destInfo->addr)
    );
    if (sendRet == -1) {
        perror("ft_ping: ");
    }
    return (sendRet);
}
