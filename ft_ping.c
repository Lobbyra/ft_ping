#include "./ft_ping.h"

static volatile int keepRunning = 1;

void intHandler(int dummy) {
    (void)dummy;
    keepRunning = 0;
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

int openSocket() {
    int sock;
    int ttl = 1;

    sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock == -1) {
        return (sock);
    }
    if (setsockopt(sock, SOL_IP, IP_TTL, &ttl, sizeof(ttl)) != 0) {
        // printf("\nSetting socket options to TTeL failed!\n");
        return (1);
    } else {
        // printf("\nSocket set to TTL...\n");
    }
    return (sock);
}

void print_hex(const char *data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        printf("%02X ", (unsigned char)data[i]);
    }
    printf("\n");
}


int ft_ping(bool isVerbose, char* host) {
    struct addrinfo* destInfo;
    struct icmphdr* packet;
    struct sockaddr_in destAddr;
    int sock;
    char rbuf[1024];
    struct sockaddr_in rAddr;
    socklen_t rAddrLen = sizeof(rAddr);

    signal(SIGINT, intHandler);
    printf("isVerbose %d, Host: %s\n", isVerbose, host);
    // Try a DNS resolution
    destInfo = getDest(host);
    if (destInfo == NULL) {
        return (1);
    }
    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family = AF_INET;
    destAddr.sin_addr = ((struct sockaddr_in*)destInfo->ai_addr)->sin_addr;
    sock = openSocket();
    if (sock == -1) {
        perror("Socket could not be created");
        freeaddrinfo(destInfo);
        return (1);
    }
    while (keepRunning) {
        packet = craftIcmpPackage();
        printf(
            "seqId = %d, Pid = %d, Checksum = %d\n",
            ntohs(packet->un.echo.sequence),
            ntohs(packet->un.echo.id),
            packet->checksum
        );
        fflush(stdout);
        if (
            sendto(
                sock,
                packet,
                sizeof(struct icmphdr),
                0,
                (struct sockaddr*)&destAddr,
                sizeof(destAddr)
            ) <= 0
        ) {
            perror("ft_ping: ");
        }
        int responseLen;
        fflush(stdout);
        memset(rbuf, 0, sizeof(rbuf));
        responseLen = recvfrom(sock, rbuf, sizeof(rbuf), 0, (struct sockaddr*)&rAddr, &rAddrLen);
        fflush(stdout);
        if (responseLen <= 0) {
            perror("No response received or error");
        } else {
            // Parse the IP and ICMP headers
            struct iphdr *ip_header = (struct iphdr*) rbuf;
            struct icmphdr *icmp_response = (struct icmphdr*) (rbuf+ (ip_header->ihl << 2));

            print_hex(rbuf, responseLen);
            // Check if it's an ICMP Echo Reply
            if (icmp_response->type == ICMP_EXC_TTL) {
                printf("%d bytes ✅ from %s\n", responseLen, inet_ntoa(rAddr.sin_addr));
            } else {
                printf("%d bytes ❌ from %s\n", responseLen, inet_ntoa(rAddr.sin_addr));
            }
            printf("type: %u, ", ntohs(icmp_response->type));
            printf("code: %u, ", ntohs(icmp_response->code));
            printf("id: %d, ", ntohs(icmp_response->un.echo.id));
            printf("seq: %d\n", ntohs(icmp_response->un.echo.sequence));
        }
        sleep(1);
        free(packet);
    }
    freeaddrinfo(destInfo);
    close(sock);
    return (0);
}
