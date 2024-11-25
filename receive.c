#include "./ft_ping.h"

int receive(struct Ping* ping) {
    int responseLen;
    char rbuf[1024];
    struct sockaddr_in rAddr;
    socklen_t rAddrLen = sizeof(rAddr);
    struct iphdr *ip_header;
    int iphdrlen;
    struct icmphdr *icmp_response;
    struct icmphdr *icmp_req = NULL;

    memset(rbuf, 0, sizeof(rbuf));
    responseLen = recvfrom(
        ping->sock,
        rbuf,
        sizeof(rbuf),
        0,
        (struct sockaddr*)&rAddr,
        &rAddrLen
    );
    if (responseLen <= 0) {
        perror("No response received or error");
        return (1);
    } else {
        // Parse the IP and ICMP headers
        ip_header = (struct iphdr*) rbuf;
        iphdrlen = ip_header->ihl << 2;
        icmp_response = (struct icmphdr*) (rbuf + iphdrlen);

        printf(
            "%d bytes from %s: icmp_seq=%d ttl=%d ",
            responseLen,
            inet_ntoa(rAddr.sin_addr),
            ping->seqId,
            ip_header->ttl
        );
        // Check if it's an ICMP Echo Reply
        if (icmp_response->type != ICMP_ECHOREPLY) {
            printf("❌❌❌❌❌❌❌");
            print_icmp_code(icmp_response->type, icmp_response->code, "");
            icmp_req = (struct icmphdr*)(rbuf + 8 + iphdrlen * 2);
            printf("REQ: \n");
            debugIcmp(icmp_req);
            printf("\n");
        }
    }
    return (0);
}
