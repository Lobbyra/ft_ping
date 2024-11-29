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

unsigned short calculateChecksum(unsigned char *addr, int len)
{
    int sum = 0;
    unsigned short answer = 0;
    unsigned short *wp;

    for (wp = (unsigned short *) addr; len > 1; wp++, len -= 2)
        sum += *wp;

    /* Take in an odd byte if present */
    if (len == 1)
        {
        *(unsigned char *) &answer = *(unsigned char *) wp;
        sum += answer;
        }
    printf("before bitswitch: ");
    print_hex((void*)&sum, sizeof(int));
    sum = (sum >> 16) + (sum & 0xffff);	/* add high 16 to low 16 */
    printf("after bitswitch: ");
    print_hex((void*)&sum, sizeof(int));
    sum += (sum >> 16);		/* add carry */
    printf("A: ");
    print_hex((void*)&sum, sizeof(int));
    answer = ~sum;		/* truncate to 16 bits */
    printf("B: ");
    print_hex((void*)&sum, sizeof(int));
    return answer;
}

