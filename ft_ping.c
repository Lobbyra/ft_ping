#include "./ft_ping.h"

enum e_pingState {SEND, RECEIVE, STOP};
enum e_pingState pingState = SEND;

void interuptHandler() { pingState = STOP; };
void alarmHandler() { pingState = SEND; };

static int initSignals() {
    struct itimerval timer;
    struct sigaction saALRM;
    struct sigaction saINT;

    // Register the SIGALRM handler
    saALRM.sa_handler = alarmHandler;
    saALRM.sa_flags = 0;
    sigemptyset(&saALRM.sa_mask);
    if (sigaction(SIGALRM, &saALRM, NULL) == -1) {
        perror("sigaction");
        return (1);
    }
    // Register the SIGINT handler
    saINT.sa_handler = interuptHandler;
    saINT.sa_flags = 0;
    sigemptyset(&saINT.sa_mask);

    if (sigaction(SIGINT, &saINT, NULL) == -1) {
        perror("sigaction");
        return (1);
    }
    // SET SIGNAL LOOP
    timer.it_value.tv_sec = LOOP_DURATION_IN_SEC;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = LOOP_DURATION_IN_SEC;
    timer.it_interval.tv_usec = 0;
    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        perror("setitimer");
        return (1);
    }
    return (0);
}

int ft_ping(bool isVerbose, char* host) {
    struct s_destInfo destInfo;
    u_int16_t seqId = 0;
    const pid_t pid = getpid();
    struct PingStats pingStats;

    memset(&pingStats, 0, sizeof(pingStats));
    if (initSignals() != 0) {
        return (1);
    }
    (void)isVerbose;
    if (getDestInfo(host, &destInfo) != 0) {
        return (1);
    }
    // PRINT WELCOME
    printf(
        "PING %s (%s): %ld data bytes",
        host,
        inet_ntoa(destInfo.addr.sin_addr),
        sizeof(struct s_ping)
    );
    if (isVerbose == true) {
        printf (", id 0x%04x = %u", pid, pid);
    }
    printf("\n");
    while (pingState != STOP) {
        if (pingState == SEND) {
            sendPacket(&destInfo, pid, seqId);
            seqId++;
            pingState = RECEIVE;
        } else if (pingState == RECEIVE) {
            receivePacket(destInfo.sockfd, pid, seqId, &pingStats);
        }
    }
    printPingStats(&pingStats, host, seqId);
    close(destInfo.sockfd);
    freeaddrinfo(destInfo.info);
    return (0);
}
