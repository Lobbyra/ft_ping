#ifndef FT_PING_H
# define FT_PING_H

// STD
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/time.h>
// NET
#include <netdb.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include "./structures.h"

#define LOOP_DURATION_IN_SEC 1
#define DEFAULT_TTL 255

int ft_ping(bool isVerbose, char* host);

int sendPacket(
    const struct s_destInfo* destInfo,
    const pid_t pid,
    const uint16_t seqId
);
int getDestInfo(char* host, struct s_destInfo* destInfo);
int receivePacket(const int sockfd, const pid_t pid, const uint16_t seqId);

double getMsDiff(struct timeval *tv1, struct timeval *tv2);
unsigned short calculateChecksum(unsigned char *addr, int len);
void print_hex(const char *data, size_t length);

#endif
