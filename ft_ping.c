#include "./ft_ping.h"

static volatile int keepRunning = 1;

void intHandler(int dummy) {
    (void)dummy;
    keepRunning = 0;
}

int ft_ping(bool isVerbose, char* host) {
    struct addrinfo* destInfo;
    struct sockaddr_in destAddr;
    struct icmphdr* packet;
    struct timeval lastRecvTv; // Last received packet time
    struct timeval receivedTv; // Time at packet reception
    struct timeval lastSendTv; // Time when the last packet was send
    struct Ping ping;
    struct timeval timeout; // Select timeout

    // INIT
    (void)isVerbose;
    signal(SIGINT, intHandler);
    // Get dest info
    destInfo = getDest(host);
    if (destInfo == NULL) {
        return (1);
    }
    setDestAddr(&destAddr, destInfo);
    // Open the socket
    ping.sock = openSocket();
    if (ping.sock == -1) {
        perror("Socket could not be created");
        freeaddrinfo(destInfo);
        return (1);
    }
    // Init time structures
    gettimeofday(&lastRecvTv, NULL);
    gettimeofday(&lastSendTv, NULL);
    // Init global data structure
    initPingStruct(&ping);
    // Craft first packet
    packet = craftIcmpPackage(&ping);
    printWelcome(host, &destAddr);
    // LOOP
    while (keepRunning) {
        // IF I SENDED FROM AT LEAST 1000ms
        if (isTimeElapsed(&lastSendTv, 1000) == true) {
        //      SEND
                sendPacket(&ping, packet, &destAddr);
                fflush(stdout);
                ping.seqId += 1;
                gettimeofday(&lastSendTv, NULL);
        }
        if (selectSock(ping.sock, &timeout) > 0) {
            // RECEIVE
            receive(&ping);
            // MEASURE TIME
            gettimeofday(&receivedTv, NULL);
            // STORE TIME
            saveStat(&lastRecvTv, &receivedTv, &timeout, &ping);
            // RESET LAST
            gettimeofday(&lastRecvTv, NULL);
        }
    }
    // END
    printPingStats(&ping);
    free(packet);
    freeaddrinfo(destInfo);
    close(ping.sock);
    return (0);
}
