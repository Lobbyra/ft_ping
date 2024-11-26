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

void printWelcome(
    struct Ping* ping,
    struct sockaddr_in* destAddr,
    bool isVerbose
) {
    printf(
        "PING %s (%s): %ld data bytes",
        ping->host,
        inet_ntoa(destAddr->sin_addr),
        sizeof(struct icmphdr)
    );
    if (isVerbose == true) {
        printf (", id 0x%04x = %u", ping->id, ping->id);
    }
    printf("\n");
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

int selectSock(int sock) {
    fd_set readfds;
    struct timespec timeout;
    sigset_t sigmask;

    // Initialize the file descriptor set
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);

    // Set a minimal time out just to get the readiness of socker reading
    timeout.tv_sec = 0;
    timeout.tv_nsec = 1;

    sigemptyset(&sigmask);
    return (pselect(sock + 1, &readfds, NULL, NULL, &timeout, &sigmask));
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
        100 - ((ping->nRecv * 100) / (ping->seqId ZERO_DIV_PROTECTOR))
    );
}
