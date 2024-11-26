
#ifndef FT_PING_H
# define FT_PING_H

// Fix some intellinsense issues
#define _XOPEN_SOURCE 700
#define __USE_MISC 1

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

#define ZERO_DIV_PROTECTOR | 1

struct Ping {
    char *host;
    double min; // Minimum diff
    double avg; // Average diff
    double max; // Max diff
    double stddev; // Total variance
    double total; // Total diff waited 
    double totalsq; // Total diff squarred used in variance compute
    int seqId; // Xth packet sended
    pid_t id; // Id of the sender, here the pid of the program
    int sock; // ICMP socket
    int nRecv; // Number of received packets
};

#define ONE_SEC_MS 1000

// GLOBAL
int ft_ping(bool isVerbose, char *host);
int sendPacket(
    struct Ping* ping,
    struct icmphdr* packet,
    struct sockaddr_in *destAddr
);
int receive(struct Ping* ping);
void saveStat(
    struct timeval* beforeTv,
    struct timeval* afterTv,
    struct Ping *ping
);

// UTILS
bool isTimeElapsed(struct timeval* source, uint16_t timeInMs);
void printWelcome(
    struct Ping* ping,
    struct sockaddr_in* destAddr,
    bool isVerbose
);
void printPingStats(struct Ping *ping);

// NETWORK
int selectSock(int sock);
int openSocket();
void setDestAddr(struct sockaddr_in* destAddr, struct addrinfo* destInfo);
struct addrinfo* getDest(char* host);

// ICMP
struct icmphdr *craftIcmpPackage(struct Ping *ping);
void initPingStruct(struct Ping* ping, char *host);



#endif
