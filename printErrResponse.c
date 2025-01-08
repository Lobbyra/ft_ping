#include "./ft_ping.h"

static void printIPHdrDump(struct iphdr* ipHdr) {
    size_t ipHdrDumpIndex;

    printf ("IP Hdr Dump:\n ");
    for (
        ipHdrDumpIndex = 0;
        ipHdrDumpIndex < sizeof (*ipHdr);
        ++ipHdrDumpIndex
    ) {
        // Group bytes two by two
        printf (
            "%02x%s",
            *((unsigned char *) ipHdr + ipHdrDumpIndex),
            (ipHdrDumpIndex % 2) ? " " : ""
        );
    }
    printf ("\n");
    printf(
        "Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src\tDst\tData\n"
    );
    // VERSION, HL, TOS
    printf(" %1x  %1x  %02x", ipHdr->version, ipHdr->ihl, ipHdr->tos);
    // LEN, ID
    printf(
        " %04x %04x",
        (ipHdr->tot_len > 0x2000) ? ntohs (ipHdr->tot_len) : ipHdr->tot_len,
        ntohs(ipHdr->id)
    );
    // FLAG, off
    printf(
        "   %1x %04x",
        (ntohs (ipHdr->frag_off) & 0xe000) >> 13,
        ntohs (ipHdr->frag_off) & 0x1fff
    );
    // TTL, pro, cks
    printf (
        "  %02x  %02x %04x",
        ipHdr->ttl,
        ipHdr->protocol,
        ntohs (ipHdr->check)
    );
    // IP SRC
    printf (" %s ", inet_ntoa (*((struct in_addr *) &ipHdr->saddr)));
    // IP DST
    printf (" %s ", inet_ntoa (*((struct in_addr *) &ipHdr->daddr)));
    printf("\n");
}

static void printICMPDump(struct icmphdr* hdr) {
    printf (
        "ICMP: type %u, code %u, size %lu",
        hdr->type,
        hdr->code,
	    sizeof(struct s_ping)
    );
    printf(
        ", id 0x%04x, seq 0x%04x",
        ntohs(hdr->un.echo.id),
        ntohs(hdr->un.echo.sequence)
    );
    printf ("\n");
}

struct icmp_code_descr icmp_code_descr[] = {
    {ICMP_DEST_UNREACH, ICMP_NET_UNREACH, "Destination Net Unreachable"},
    {ICMP_DEST_UNREACH, ICMP_HOST_UNREACH, "Destination Host Unreachable"},
    {ICMP_DEST_UNREACH, ICMP_PROT_UNREACH, "Destination Protocol Unreachable"},
    {ICMP_DEST_UNREACH, ICMP_PORT_UNREACH, "Destination Port Unreachable"},
    {ICMP_DEST_UNREACH, ICMP_FRAG_NEEDED, "Fragmentation needed and DF set"},
    {ICMP_DEST_UNREACH, ICMP_SR_FAILED, "Source Route Failed"},
    {ICMP_DEST_UNREACH, ICMP_NET_UNKNOWN, "Network Unknown"},
    {ICMP_DEST_UNREACH, ICMP_HOST_UNKNOWN, "Host Unknown"},
    {ICMP_DEST_UNREACH, ICMP_HOST_ISOLATED, "Host Isolated"},
    {ICMP_DEST_UNREACH, ICMP_NET_UNR_TOS, "Destination Network Unreachable At This TOS"},
    {ICMP_DEST_UNREACH, ICMP_HOST_UNR_TOS, "Destination Host Unreachable At This TOS"},
    {ICMP_REDIRECT, ICMP_REDIR_NET, "Redirect Network"},
    {ICMP_REDIRECT, ICMP_REDIR_HOST, "Redirect Host"},
    {ICMP_REDIRECT, ICMP_REDIR_NETTOS, "Redirect Type of Service and Network"},
    {ICMP_REDIRECT, ICMP_REDIR_HOSTTOS, "Redirect Type of Service and Host"},
    {ICMP_TIME_EXCEEDED, ICMP_EXC_TTL, "Time to live exceeded"},
    {ICMP_TIME_EXCEEDED, ICMP_EXC_FRAGTIME, "Frag reassembly time exceeded"}
};

char* getErrorStr(uint8_t type, uint8_t code) {
    int i;

    for(i = 0; i < (int)sizeof(icmp_code_descr); i++) {
        if (
            icmp_code_descr[i].code == code &&
            icmp_code_descr[i].type == type
        ) {
            return (icmp_code_descr[i].diag);
        }
    }
    return (NULL);
}

void printErrResponse(
    const pid_t pid,
    const bool isVerbose,
    const struct sockaddr_in* rAddr,
    const char* buf,
    const size_t bufSize
) {
    struct iphdr* destIPHdr;
    struct iphdr* srcIPHdr;
    struct s_ping* ogPing;
    struct icmphdr* response;
    char* errorMsg;

    destIPHdr = (struct iphdr*)buf;
    srcIPHdr = (struct iphdr*)(buf + 8 + (destIPHdr->ihl * 4));
    response = (struct icmphdr*)(buf + (destIPHdr->ihl * 4));
    ogPing = (struct s_ping*)(  
        buf + 8 + (destIPHdr->ihl * 4) + (srcIPHdr->ihl * 4)
    );
    errorMsg = getErrorStr(response->type, response->code);
    if (ntohs(ogPing->header.un.echo.id) != pid) {
        return;
    }
    printf(
        "%ld bytes from %s: %s\n",
        bufSize,
        inet_ntoa(rAddr->sin_addr),
        errorMsg ? errorMsg : "UNKNOWN ERROR"
    );
    if (isVerbose == true) {
        if (
            response->type == ICMP_DEST_UNREACH ||
            response->type == ICMP_REDIRECT ||
            response->type == ICMP_TIME_EXCEEDED
        ) {
            printIPHdrDump(srcIPHdr);
            printICMPDump(&(ogPing->header));
        }
    }
}
