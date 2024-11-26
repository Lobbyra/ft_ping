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
    struct timeval receivedTv; // Time at packet reception
    struct timeval lastSendTv; // Time when the last packet was send
    struct Ping ping; // Wrap of many ping info

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
    // Init global data structure
    initPingStruct(&ping, host);
    // Craft first packet
    packet = craftIcmpPackage(&ping);
    printWelcome(&ping, &destAddr, isVerbose);
    // Init time structures
    gettimeofday(&lastSendTv, NULL);
    // LOOP
    while (keepRunning) {
        // IF I SENDED FROM AT LEAST 1000ms
        if (isTimeElapsed(&lastSendTv, ONE_SEC_MS) == true || ping.seqId == 0) {
            //  SEND
            sendPacket(&ping, packet, &destAddr);
            ping.seqId += 1;
            gettimeofday(&lastSendTv, NULL);
        }
        if (selectSock(ping.sock) == 1) {
            // RECEIVE
            if (receive(&ping) == -1) {
                continue;
            }
            // MEASURE TIME
            gettimeofday(&receivedTv, NULL);
            // STORE TIME
            saveStat(&lastSendTv, &receivedTv, &ping);
            // RESET LAST
            ping.nRecv += 1;
        }
    }
    // END
    printPingStats(&ping);
    free(packet);
    freeaddrinfo(destInfo);
    close(ping.sock);
    return (0);
}
