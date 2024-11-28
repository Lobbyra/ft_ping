#include "./ft_ping.h"

enum e_pingState {SEND, RECEIVE, STOP};
enum e_pingState pingState = SEND;

void interuptHandler() { pingState = STOP; };
void alarmHandler() { pingState = SEND; };

static void initSignals() {
    struct itimerval timer;

    // SET SIGNAL HANDLER
    signal(SIGALRM, alarmHandler);
    signal(SIGINT, interuptHandler);
    // SET SIGNAL LOOP
    timer.it_value.tv_sec = LOOP_DURATION_IN_SEC;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = LOOP_DURATION_IN_SEC;
    timer.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &timer, NULL);
}

int ft_ping(bool isVerbose, char* host) {
    struct s_destInfo destInfo;
    size_t seqId = 0;
    const pid_t pid = getpid();

    initSignals();
    (void)isVerbose;
    if (getDestInfo(host, &destInfo) != 0) {
        return (1);
    }
    while (pingState != STOP) {
        if (pingState == SEND) {
            printf("SEND to %s\n", inet_ntoa(destInfo.addr.sin_addr));
            fflush(stdout);
            sendPacket(&destInfo, pid, seqId);
            pingState = RECEIVE;
        } else if (pingState == RECEIVE) {
            pause();
        }
    }
    freeaddrinfo(destInfo.info);
    return (0);
}
