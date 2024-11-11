#include "ft_ping.h"

void print_hex(const char *data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        printf("%02X ", (unsigned char)data[i]);
    }
    printf("\n");
}

#define NITEMS(a) sizeof(a)/sizeof((a)[0])

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

void print_icmp_code (int type, int code, char *prefix) {
    struct icmp_code_descr *p;

    for (
        p = icmp_code_descr;
        p < icmp_code_descr + NITEMS (icmp_code_descr);
        p++
    ) {
        if (p->type == type && p->code == code)
        {
            printf ("%s\n", p->diag);
            return;
        }
    }
    printf ("%s, Unknown Code: %d\n", prefix, code);
}

void debugIcmp(struct icmphdr *i) {
    printf("type: %d, ", i->type);
    printf("code: %d, ", i->code);
    printf("id: %d, ", ntohs(i->un.echo.id));
    printf("seq: %d\n", ntohs(i->un.echo.sequence));
    printf("ICMP dump: \n");
    print_hex((char*)(i), 8);
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

    // REVERSE DNS DEAD CODE
    // /* loop over all returned results and do inverse lookup */
    // for (res = result; res != NULL; res = res->ai_next) {   
    //     char hostname[NI_MAXHOST];
    //     error = getnameinfo(res->ai_addr, res->ai_addrlen, hostname, NI_MAXHOST, NULL, 0, 0); 
    //     if (error != 0) {
    //         fprintf(stderr, "error in getnameinfo: %s\n", gai_strerror(error));
    //         continue;
    //     }
    //     if (*hostname != '\0')
    //         printf("hostname: %s\n", hostname);
    // }
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
