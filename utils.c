#include "./ft_ping.h"

int openSocket() {
    int sock;
    int ttl = 201;

    sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock == -1) {
        return (sock);
    }
    if (setsockopt(sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) != 0) {
        // printf("\nSetting socket options to TTeL failed!\n");
        return (1);
    } else {
        // printf("\nSocket set to TTL...\n");
    }
    return (sock);
}

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

struct icmphdr *craftIcmpPackage(struct Ping *ping) {
    struct icmphdr *packet;
    packet = malloc(sizeof(struct icmphdr));
    // Store in raw bytes but trick the compiler to treat as struct
    struct icmphdr *icmp = (struct icmphdr *) packet;

    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->un.echo.id = htons(ping->id);
    icmp->un.echo.sequence = htons(ping->seqId);
    icmp->checksum = 0;
    icmp->checksum = calculateChecksum(packet, sizeof(struct icmphdr));
    return (packet);
}

struct addrinfo* getDest(char* host) {
    struct addrinfo* result;
    struct addrinfo hints; // Wanted address type (IPv4)
    int error;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = IPPROTO_ICMP;
    /* resolve the domain name into a list of addresses */
    error = getaddrinfo(host, NULL, &hints, &result);
    if (error != 0) {
        fprintf(stderr, "ft_ping: %s: %s\n", host, gai_strerror(error));
        return (NULL);
    }
    return (result);
}

void setDestAddr(struct sockaddr_in* destAddr, struct addrinfo* destInfo) {
    memset(destAddr, 0, sizeof(*destAddr));
    destAddr->sin_family = AF_INET;
    destAddr->sin_addr = ((struct sockaddr_in*)destInfo->ai_addr)->sin_addr;
}

void printWelcome(char* host, struct sockaddr_in* destAddr) {
    printf(
        "PING %s (%s): %ld data bytes\n",
        host,
        inet_ntoa(destAddr->sin_addr),
        sizeof(struct icmphdr)
    );
}

void initPingStruct(struct Ping* ping, char* host) {
    ping->host = host;
    ping->min = 0;
    ping->avg = 0;
    ping->max = 0;
    ping->stddev = 0;
    ping->total = 0;
    ping->id = getpid();
    ping->seqId = 0;
    ping->nRecv = 0;
}

// Return true if
bool isTimeElapsed(struct timeval* source, uint16_t timeInMs) {
    struct timeval now;
    double nowMicroSec;
    double sourceMicroSec;
    double diffMicroSec;

    gettimeofday(&now, NULL);
    nowMicroSec = (double)now.tv_sec * 1000000 + now.tv_usec;
    sourceMicroSec = (double)source->tv_sec * 1000000 + source->tv_usec;
    diffMicroSec = (nowMicroSec - sourceMicroSec);
    fflush(stdout);
    return (diffMicroSec / 1000 > timeInMs);
}

int selectSock(int sock, struct timeval* timeout) {
    fd_set readfds;
    // sigset_t sigmask;

    // Initialize the file descriptor set
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);  // Monitor standard input (fd = 0)

    // Set a timeout of 5 seconds
    timeout->tv_sec = 0;
    timeout->tv_usec = 10;

    // Block no signals (sigmask = empty set)
    // sigemptyset(&sigmask);
    return (select(sock + 1, &readfds, NULL, NULL, timeout));
}

struct icmp_code_descr
{
  int type;
  int code;
  char *diag;
} icmp_code_descr[] = {
    {ICMP_DEST_UNREACH, ICMP_NET_UNREACH, "Destination Net Unreachable"},
    {ICMP_DEST_UNREACH, ICMP_HOST_UNREACH, "Destination Host Unreachable"},
    {ICMP_DEST_UNREACH, ICMP_PROT_UNREACH, "Destination Protocol Unreachable"},
    {ICMP_DEST_UNREACH, ICMP_PORT_UNREACH, "Destination Port Unreachable"},
    {ICMP_DEST_UNREACH, ICMP_FRAG_NEEDED, "Fragmentation needed and DF set"},
    {ICMP_DEST_UNREACH, ICMP_SR_FAILED, "Source Route Failed"},
    {ICMP_DEST_UNREACH, ICMP_NET_UNKNOWN, "Network Unknown"},
    {ICMP_DEST_UNREACH, ICMP_HOST_UNKNOWN, "Host Unknown"},
    {ICMP_DEST_UNREACH, ICMP_HOST_ISOLATED, "Host Isolated"},
    {
        ICMP_DEST_UNREACH, ICMP_NET_UNR_TOS,
        "Destination Network Unreachable At This TOS"
    },
    {
        ICMP_DEST_UNREACH, ICMP_HOST_UNR_TOS,
        "Destination Host Unreachable At This TOS"
    },
    {ICMP_REDIRECT, ICMP_REDIR_NET, "Redirect Network"},
    {ICMP_REDIRECT, ICMP_REDIR_HOST, "Redirect Host"},
    {ICMP_REDIRECT, ICMP_REDIR_NETTOS, "Redirect Type of Service and Network"},
    {ICMP_REDIRECT, ICMP_REDIR_HOSTTOS, "Redirect Type of Service and Host"},
    {ICMP_TIME_EXCEEDED, ICMP_EXC_TTL, "Time to live exceeded"},
    {ICMP_TIME_EXCEEDED, ICMP_EXC_FRAGTIME, "Frag reassembly time exceeded"}
};

void print_icmp_code(int type, int code, char *prefix) {
    struct icmp_code_descr *p;

    for (
        p = icmp_code_descr;
        p < icmp_code_descr + NITEMS (icmp_code_descr);
        p++
    ) {
        if (p->type == type && p->code == code) {
            printf("%s\n", p->diag);
            return;
        }
    }
    printf("%s, Unknown Code: %d\n", prefix, code);
}

void debugIcmp(struct icmphdr *i) {
    printf("type: %d, ", i->type);
    printf("code: %d, ", i->code);
    printf("id: %d, ", ntohs(i->un.echo.id));
    printf("seq: %d\n", ntohs(i->un.echo.sequence));
    printf("ICMP dump: \n");
    print_hex((char*)(i), 8);
}

void print_hex(const char *data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        printf("%02X ", (unsigned char)data[i]);
    }
    printf("\n");
}

void saveStat(
    struct timeval* beforeTv,
    struct timeval* afterTv,
    struct Ping *ping
) {
    double beforeMicroSec = (
        (double)beforeTv->tv_sec * 1000000 + beforeTv->tv_usec
    );
    double afterMicroSec = (
        (double)afterTv->tv_sec * 1000000 + afterTv->tv_usec
    );
    double diffMsSec = (afterMicroSec - beforeMicroSec) / 1000;
    ping->total += diffMsSec;
    ping->totalsq += diffMsSec * diffMsSec;
    if (ping->seqId == 0) {
        ping->avg = ping->total;
    } else {
        ping->avg = ping->total / (double)ping->seqId;
    }
    if (ping->min == 0 || ping->min > diffMsSec) {
        ping->min = diffMsSec;
    }
    if (ping->max == 0 || ping->max < diffMsSec) {
        ping->max = diffMsSec;
    }
    printf("time=%.3f ms\n", diffMsSec);
    ping->stddev = ping->totalsq / (double)ping->seqId - ping->avg * ping->avg;
}



static double nabs(double a) {
    if (a < 0) {
        return (a);
    } else {
        return (a * -1);
    }
}

double nsqrt(double a, double prec) {
    double x0;
    double x1;

    if (a < 0) {
        return 0;
    }
    if (a < prec) {
        return 0;
    }
    x1 = a / 2;
    x0 = x1;
    while (nabs (x1 - x0) > prec)
    {
        x0 = x1;
        x1 = (x0 + a / x0) / 2;
    }
    return x1;
}

void printPingStats(struct Ping *ping) {
    printf("--- %s ping statistics ---\n", ping->host);
    if (ping->nRecv > 0) {
        printf (
            "round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
            ping->min,
            ping->avg,
            ping->max,
            nsqrt (ping->stddev, 0.0005)
        );
    }
    printf(
        "%d packets transmitted, %d packets received, %d%% packet loss\n",
        ping->seqId,
        ping->nRecv,
        100 - ((ping->nRecv * 100) / ping->seqId)
    );
}
