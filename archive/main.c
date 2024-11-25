#include "./ft_ping.h"

void help() {
    printf("Usage: ft_ping [-v] [-?] HOST\n");
    printf("Send ICMP ECHO_REQUEST packets to network hosts.\n");
}

int main(int argc, char **argv) {
    bool isVerbose = false;

    if (argc < 2) {
        fprintf(stderr, "ping: missing host operand\n");
        return (1);
    }
    if (strcmp("-v", argv[1]) == 0) {
        if (argc < 3) {
            fprintf(stderr, "ping: missing host operand\n");
            return (1);
        }
        isVerbose = true;
    } else if (strcmp("-?", argv[1]) == 0) {
        help();
        return (0);
    }
    return (ft_ping(isVerbose, argv[isVerbose ? 2 : 1]));
}
