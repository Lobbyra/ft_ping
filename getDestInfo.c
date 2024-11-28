#include "./ft_ping.h"

static int openSocket() {
    int sock;
    int ttl = DEFAULT_TTL;

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

int getDestInfo(char* host, struct s_destInfo* destInfo) {
    struct addrinfo* result;
    struct addrinfo hints; // Wanted address type (IPv4)
    int error;

    memset(destInfo, 0, sizeof(*destInfo));
    destInfo->host = host;
    // Get addrinfo
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = IPPROTO_ICMP;
    error = getaddrinfo(host, NULL, &hints, &result);
    if (error != 0) {
        fprintf(stderr, "ft_ping: %s: %s\n", host, gai_strerror(error));
        return (1);
    }
    destInfo->info = result;
    // SET DEST ADDR
    memset(&(destInfo->addr), 0, sizeof(destInfo->addr));
    destInfo->addr.sin_family = AF_INET;
    destInfo->addr.sin_addr = (
        ((struct sockaddr_in*)destInfo->info->ai_addr)->sin_addr
    );
    // OPEN SOCKET
    destInfo->sockfd = openSocket();
    if (destInfo->sockfd == -1) {
        fprintf(stderr, "ft_ping: %s: %s\n", host, strerror(errno));
        return (1);
    }
    return (0);
}
