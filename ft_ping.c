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
    initSignals();
    while (pingState != STOP) {
        if (pingState == SEND) {
            printf("SEND\n");
            fflush(stdout);
            pingState = RECEIVE;
        } else if (pingState == RECEIVE) {
            pause();
        }
    }
    return (0);
}
