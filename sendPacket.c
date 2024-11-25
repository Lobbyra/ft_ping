#include "./ft_ping.h"

int sendPacket(
    struct Ping* ping,
    struct icmphdr* packet,
    struct sockaddr_in *destAddr
) {
    if (
        sendto(
            ping->sock,
            packet,
            sizeof(struct icmphdr),
            0,
            (struct sockaddr*)destAddr,
            sizeof(*destAddr)
        ) <= 0
    ) {
        perror("ft_ping: ");
    }
    return (0);
}
