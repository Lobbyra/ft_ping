
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
void print_hex(const char *data, size_t length);
void print_icmp_code (int type, int code, char *prefix);
void debugIcmp(struct icmphdr *i);
struct addrinfo* getDest(char* host);
double nsqrt (double a, double prec);

int ft_ping(bool isVerbose, char *host);

// icmp.h
struct icmphdr *craftIcmpPackage(u_int16_t seqId);


#endif
