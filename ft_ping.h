
#ifndef FT_PING_H
# define FT_PING_H

// Fix some intellinsense issues
#define _XOPEN_SOURCE 700

// STD
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

// NET
#include <netdb.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

int ft_ping(bool isVerbose, char *host);

// icmp.h
struct icmphdr *craftIcmpPackage();

#endif
