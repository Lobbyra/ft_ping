#include "ft_ping.h"

// Calculate the checksum (RFC 1071)
unsigned short calculateChecksum(void *packet, int len) {
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

struct icmphdr *craftIcmpPackage() {
    static u_int16_t seqId = 0;
    struct icmphdr *packet;
    packet = malloc(sizeof(struct icmphdr));
    // Store in raw bytes but trick the compiler to treat as struct
    struct icmphdr *icmp = (struct icmphdr *) packet;

    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->un.echo.id = htons(getpid());
    icmp->un.echo.sequence = htons(seqId);
    icmp->checksum = 0;
    icmp->checksum = calculateChecksum(packet, sizeof(struct icmphdr));
    seqId += 1;
    return (packet);
}
