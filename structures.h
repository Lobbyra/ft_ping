
#ifndef STRUCTURES_H
# define STRUCTURES_H

#include "./ft_ping.h"

struct s_destInfo {
    char* host;
    int sockfd;
    struct addrinfo* info;
    struct sockaddr_in addr;
};

struct s_ping {
    struct icmphdr header;
    struct timeval sentTimestamp;
    u_int32_t padding;
};

struct s_errPing {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint32_t unused;
    struct iphdr ipHeader;
    struct s_ping srcPing;
};

#endif
