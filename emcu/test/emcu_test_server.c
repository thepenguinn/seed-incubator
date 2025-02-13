#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define IP_ADDR "127.0.0.1"
#define PORT    10898
#define BACKLOG    10

int create_tcp_server();

int create_tcp_server() {

    int sockfd, connfd;
    struct sockaddr_in server_addr, conn_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1) {
        printf("socket() failed.\n");
        return -1;
    }
    printf("Created socket.\n");

    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        printf("setsockopt() failed.\n");
    }
    printf("Setted socket to reuseadder.\n");

    bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(IP_ADDR);
    server_addr.sin_port = (uint16_t) htons(PORT);

    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        printf("bind() failed.\n");
        return -1;
    }
    printf("Binded successfully.\n");

    if (listen(sockfd, BACKLOG) == -1) {
        printf("listen() failed.\n");
        return -1;
    }
    printf("Listening.\n");

    // now accept connection
    unsigned int len = sizeof(conn_addr);
    connfd = accept(sockfd, (struct sockaddr *) &conn_addr, &len);

    if (connfd < 0) {
        printf("accpet() failed.\n");
        return -1;
    }
    printf("Got a connection.\n");

    return connfd;

}

int main (int argc, char *argv[]) {

    printf("hello world\n");

    int connfd = create_tcp_server();

    return 0;

}
