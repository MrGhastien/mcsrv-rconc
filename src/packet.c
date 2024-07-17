#include "packet.h"
#include "context.h"

#include <stdio.h>
#include <sys/socket.h>

static int pktCount = 1;

static char* types[4] = {
    [PKT_COMMAND_RESPONSE] = "COMMAND_RESPONSE",
    [PKT_UNKNOWN] = "UNKNOWN",
    [PKT_COMMAND] = "COMMAND",
    [PKT_LOGIN] = "LOGIN"
};

static char* pkttype(enum pkttype type) {
    if(type > PKT_LOGIN || type < PKT_COMMAND_RESPONSE)
        return types[PKT_UNKNOWN];
    return types[type];
}

static int createRequestId() {
    return pktCount++;
}

void memcpy(char* dst, const char* src, size_t len) {
    for (size_t i = 0; i < len; i++) {
        dst[i] = src[i];
    }
}

size_t strlen(const char* str) {
    if (!str)
        return 0;

    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

void pktinit(packet* packet, enum pkttype type, char* payload) {
    size_t payloadLen = strlen(payload);
    if(payloadLen >= PAYLOAD_SIZE)
        return;
    packet->type = type;
    packet->requestID = createRequestId();
    memcpy(packet->payload, payload, payloadLen); //Copy the null character too !
    packet->payload[payloadLen] = 0;
    packet->length = payloadLen + sizeof(int) * 2 + 2;
}

int pktsend(int sockfd, const packet* packet) {
    ssize_t res = send(sockfd, packet, packet->length + sizeof(int), 0);
    if (res < packet->length)
        return 0;
    if(verbose)
        printf("C -> \"%s\" [%s]\n", packet->payload, pkttype(packet->type));
    return 1;
}

int pktrecv(int sockfd, packet* outPacket) {
    size_t intsize = sizeof outPacket->length;
    ssize_t totalLen = recv(sockfd, &outPacket->length, intsize, 0);
    if (totalLen < (ssize_t)intsize) {
        return 0;
    }

    totalLen = 0;
    while (totalLen < outPacket->length - 1) {
        int res = recv(sockfd, &outPacket->requestID + totalLen, outPacket->length - totalLen - 1, 0);
        if(res <= 0)
            return 0;
        totalLen += res;
    }
    char nullterm;
    if(recv(sockfd, &nullterm, 1, 0) != 1)
        return 0;
    if(verbose)
        printf("S -> \"%s\" [%s]\n", outPacket->payload, pkttype(outPacket->type));
    return 1;
}

void pktprint(const packet* pkt) {
    printf("Size : %u bytes\n", pkt->length);
    printf("Request ID : %x\n", pkt->requestID);
    printf("Type : %s\n", pkttype(pkt->type));
    printf("Payload : \"%s\"\n", pkt->payload);
    puts("======= END OF PACKET =======");
    for (size_t i = 0; i < pkt->length + sizeof(int) - 1; i++) {
        printf("%02x ", ((char*)pkt)[i]);
    }
    putchar('\n');
    puts("======= ============= =======");
}
