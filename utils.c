#include "./ft_ping.h"

double getMsDiff(struct timeval *tv1, struct timeval *tv2) {
    double tv1MicroSec = (
        (double)tv1->tv_sec * 1000000 + tv1->tv_usec
    );
    double tv2MicroSec = (
        (double)tv2->tv_sec * 1000000 + tv2->tv_usec
    );
    double diffMsSec = (tv2MicroSec - tv1MicroSec) / 1000;
    return (diffMsSec);
}

void print_hex(const char *data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        printf("%02X ", (unsigned char)data[i]);
    }
    printf("\n");
}

unsigned short calculateChecksum(unsigned char *addr, int len) {
    int sum = 0;
    unsigned short answer = 0;
    unsigned short *wp;

    for (wp = (unsigned short *) addr; len > 1; wp++, len -= 2) {
        sum += *wp;
    }
    if (len == 1) {
        *(unsigned char *)&answer = *(unsigned char *)wp;
        sum += answer;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
    return answer;
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

void printPingStats(
    struct PingStats* pingStats,
    char* host,
    u_int16_t seqId
) {
    printf("--- %s ping statistics ---\n", host);
    printf(
        "%d packets transmitted, %d packets received, %d%% packet loss\n",
        seqId,
        pingStats->receivedPaquet,
        100 - ((pingStats->receivedPaquet * 100) / (seqId))
    );
    if (pingStats->receivedPaquet > 0) {
        printf (
            "round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
            pingStats->min,
            pingStats->avg,
            pingStats->max,
            nsqrt(pingStats->stddev, 0.0005)
        );
    }
}
