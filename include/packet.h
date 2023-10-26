#ifndef PACKET_H
#define PACKET_H

#include <stddef.h>

#define PAYLOAD_SIZE 4096

enum pkttype {
    PKT_COMMAND_RESPONSE = 0,
    PKT_UNKNOWN = 1,
    PKT_COMMAND = 2,
    PKT_LOGIN = 3
};


typedef struct pkt {
    int length;
    int requestID;
    int type;
    char payload[PAYLOAD_SIZE];
} packet;

void memcpy(char* dst, const char* src, size_t len);

size_t strlen(const char* str);

void pktinit(packet* packet, enum pkttype type, char* payload);

int pktsend(int sockfd, const packet* packet);

int pktrecv(int sockfd, packet* outPacket);

void pktprint(const packet* pkt);

#endif /* ! PACKET_H*/
