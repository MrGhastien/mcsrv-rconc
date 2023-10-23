#include <err.h>
#include <stdio.h>
#include <sys/socket.h>
#include <strings.h>

int main(int argc, char* argv[]) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        err(1, "%s", "Could not create socket.");
    }
    struct sockaddr address;
    return 0;
}
