
#ifndef FT_PING_H
# define FT_PING_H

// Fix some intellinsense issues
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

#define LOOP_DURATION_IN_SEC 1

int ft_ping(bool isVerbose, char* host);

#endif
