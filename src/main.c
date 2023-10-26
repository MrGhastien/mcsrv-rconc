#include "packet.h"
#include "context.h"

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <unistd.h>

#define BUFFER_SIZE 8192

int verbose;

static ssize_t readline(char* buf, size_t bufSize, FILE* stream) {
    int c;
    size_t len = 0;
    while ((c = fgetc(stream)) != EOF && len < bufSize) {
        if(c == '\n')
            break;
        buf[len] = c;
        len++;
    }
    buf[len] = 0;
    return len;
}

void cmdsend(int sockfd, char* cmd) {
    packet pkt;
    pktinit(&pkt, PKT_COMMAND, cmd);
    pktsend(sockfd, &pkt);

    pktinit(&pkt, PKT_COMMAND_RESPONSE, "");
    pktsend(sockfd, &pkt);
}

static int memeq(const char* a, const char* b, size_t size) {
    for (size_t i = 0; i < size; i++) {
        if(b[i] != a[i])
            return 0;
    }
    return 1;
}

int cmdrecv(int sockfd, char* responseBuffer) {
    packet pkt;
    size_t len = 0;
    while (pktrecv(sockfd, &pkt)) {
        char* endingPayload = "Unknown request 0";
        if (memeq(pkt.payload, endingPayload, strlen(endingPayload))) {
            responseBuffer[len] = 0;
            return 1;
        }
        size_t payloadLen = strlen(pkt.payload);
        memcpy(responseBuffer + len, pkt.payload,payloadLen);
        len += payloadLen;
    }
    return 0;
}

static int authenticate(int sockfd) {
    packet pkt;
    pkt.requestID = -1;
    char buf[BUFFER_SIZE];
    while(pkt.requestID == -1) {
        fputs("Enter password: ", stdout);
        readline(buf, BUFFER_SIZE, stdin);

        pktinit(&pkt, PKT_LOGIN, buf);
        pktsend(sockfd, &pkt);

        if (!pktrecv(sockfd, &pkt)) {
            puts("Failed to read packet.");
            return 0;
        }

        if (pkt.requestID == -1) {
            fputs("Incorrect password.\n", stderr);
        }
    }
    return 1;
}

static int initConnection() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        err(1, "%s", "Could not create socket.");
    }
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(25575);
    address.sin_addr.s_addr = INADDR_ANY;

    int result = connect(sockfd, (struct sockaddr*) &address, sizeof(address));

    if(result == -1) {
        err(2, "%s", "Could not connect to server");
    }
    return sockfd;
}

int main(int argc, char* argv[]) {

    for (int i = 1; i < argc; i++) {
        if(memeq(argv[i], "-v", strlen(argv[i])))
            verbose = 1;
        else {
            err(64, "Unknown option '%s'", argv[i]);
            return 64;
        }
    }

    int sockfd = initConnection();
    
    if(!authenticate(sockfd))
        return 1;
   
    int shouldRun = 1;
    char request[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    ssize_t reqLen;
    while(shouldRun) {
        fputs("$> ", stdout);
        reqLen = readline(request, BUFFER_SIZE, stdin);
        if (reqLen <= 0) {
            shouldRun = 0;
            break;
        }
        cmdsend(sockfd, request);
        if (!cmdrecv(sockfd, response)) {
            // Error or EOF
            shouldRun = 0;
            break;
        }
        printf("Reply from server : %s\n", response);
    }
    close(sockfd);
    return 0;
}
