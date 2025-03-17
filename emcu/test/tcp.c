#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

#include "tcp.h"

uint32_t tcp_recieve_uint32_t(int serverfd) {

    char buf[4];
    int i, len;
    uint32_t data;

    for (i = 0; i < 4; i++) {

        len = recv(serverfd, buf + i, 1, 0);
        if (len < 0) {
            printf("Error occurred during receiving: errno %d\n", errno);
            return 0;
        } else if (len == 0) {
            printf("Connection closed\n");
            return 0;
        }

    }

    data = (uint32_t) buf[0]
        | (((uint32_t) buf[1]) << 8)
        | (((uint32_t) buf[2]) << 16)
        | (((uint32_t) buf[3]) << 24);


    return data;

}

int tcp_send_packet(int serverfd, uint8_t cmd, uint32_t data) {

    char buf;
    int i;

    /*
     * data should be send in little endian
     * */

    buf = (char) cmd;
    send(serverfd, &buf, 1, 0);
    for (i = 0; i < 4; i++) {
        buf = (char) ((data >> i * 8) & 0xff);
        send(serverfd, &buf, 1, 0);
    }

    return 0;

}

int tcp_connect_to_server(const char *ip_addr, int port) {

    int sockfd;

    printf("%s\n", ip_addr);

    struct sockaddr_in server_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket() failed.\n");
        return -1;
    }
    printf("Created socket.\n");
    bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_addr);
    server_addr.sin_port = (uint16_t) htons(port);

    if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        printf("connect() failed.\n");
        return -1;
    }
    printf("Connected successfully.\n");

    return sockfd;

}
