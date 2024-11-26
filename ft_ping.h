
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

// UTILS.c
int openSocket();
// void print_hex(const char *data, size_t length);
// void print_icmp_code (int type, int code, char *prefix);
// void debugIcmp(struct icmphdr *i);
// struct addrinfo* getDest(char* host);
// double nsqrt (double a, double prec);
void setDestAddr(struct sockaddr_in* destAddr, struct addrinfo* destInfo);
void printWelcome(char* host, struct sockaddr_in* destAddr);

int ft_ping(bool isVerbose, char *host);

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

// icmp.h
struct icmphdr *craftIcmpPackage(struct Ping *ping);
struct addrinfo* getDest(char* host);
void initPingStruct(struct Ping* ping, char *host);
bool isTimeElapsed(struct timeval* source, uint16_t timeInMs);
int selectSock(int sock, struct timeval* timeout);
int receive(struct Ping* ping);

int sendPacket(
    struct Ping* ping,
    struct icmphdr* packet,
    struct sockaddr_in *destAddr
);
void print_icmp_code(int type, int code, char *prefix);
void debugIcmp(struct icmphdr *i);
void print_hex(const char *data, size_t length);

#define NITEMS(a) sizeof(a)/sizeof((a)[0])

void saveStat(
    struct timeval* beforeTv,
    struct timeval* afterTv,
    struct Ping *ping
);
void printPingStats(struct Ping *ping);


#endif
