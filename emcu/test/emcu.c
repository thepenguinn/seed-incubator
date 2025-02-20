#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "utils.h"
#include "emcu.h"

static const char *sub_cmd_str[SUB_CMD_END] = {
    [SUB_CMD_UNKNOWN] = "unknown",
    [SUB_CMD_HMODE]   = "hmode",
    [SUB_CMD_CMODE]   = "cmode",
    [SUB_CMD_MONITOR] = "monitor",
    [SUB_CMD_FANS]    = "fans",
    [SUB_CMD_PELTIER] = "peltier",
};

static char ip_addr[128];

static int serverfd;

int emcu(int command, int state);
int parse_args(int argc, char *argv[], int *enable);
int print_help(void);
int get_ip_addr(void);

int emcu_exhaust(int mode);
int emcu_monitor();
int emcu_fans(int state);
int emcu_peltier(int state);

int connect_to_server();

int connect_to_server() {

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
    server_addr.sin_port = (uint16_t) htons(PORT);

    if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        printf("connect() failed.\n");
        return -1;
    }
    printf("Connected successfully.\n");

    return sockfd;

}

int emcu_exhaust(int mode) {
    return 0;
}

int emcu_monitor() {

    char msg = (char) SUB_CMD_MONITOR;
    int i = 0;
    char buf[7];

    while (1) {
        send(serverfd, &msg, 1, 0);
        send(serverfd, &msg, 1, 0);

        for (i = 0; i < 6; i++) {
            recv(serverfd, buf + i, 1, 0);
        }
        buf[6] = '\0';
        printf("Temp: %s ", buf);

        for (i = 0; i < 6; i++) {
            recv(serverfd, buf + i, 1, 0);
        }
        buf[6] = '\0';
        printf("Hume: %s ", buf);

        for (i = 0; i < 6; i++) {
            recv(serverfd, buf + i, 1, 0);
        }
        buf[6] = '\0';
        printf("Voltage: %smV\n", buf);
        usleep(500);
    }

    return 0;
}

int emcu_fans(int state) {
    return 0;
}

int emcu_peltier(int state) {
    return 0;
}

int emcu (int command, int state) {

    printf("%d, %d \n", command, state);

    switch(command) {

        case SUB_CMD_HMODE:
            return emcu_exhaust(EXHAUST_HMODE);
        case SUB_CMD_CMODE:
            return emcu_exhaust(EXHAUST_CMODE);
        case SUB_CMD_MONITOR:
            return emcu_monitor();
        case SUB_CMD_FANS:
            return emcu_fans(state);
        case SUB_CMD_PELTIER:
            return emcu_peltier(state);
        default:
            return 1;

    }

    return 0;

}

int parse_args (int argc, char *argv[], int *enable) {

    int i;
    char *subcmd;
    int subcmdint = SUB_CMD_UNKNOWN;

    if (argc < 2) {
        return SUB_CMD_UNKNOWN;
    }

    subcmd = argv[1];

    for (i = 0; i < SUB_CMD_END; i++) {
        if (!strcmp(subcmd, sub_cmd_str[i])) {
            subcmdint = i;
            break;
        }
    }

    if (enable) {
        if (subcmdint == SUB_CMD_FANS || subcmdint == SUB_CMD_PELTIER) {
            if (argc >= 3) {
                if (!strcmp(argv[2], SUB_CMD_STR_ENABLE)) {
                    *enable = 1;
                } else if (!strcmp(argv[2], SUB_CMD_STR_DISABLE)) {
                    *enable = 0;
                } else {
                    return SUB_CMD_UNKNOWN;
                }
            } else {
                return SUB_CMD_UNKNOWN;
            }

        }
    }

    return subcmdint;

}

int print_help (void) {

    printf("usage: emcu [subcommand]\n");
    printf("\n");
    printf("  subcommands\n");
    printf("\n");
    printf("    hmode:   set the thermal system into heater mode.\n");
    printf("    cmode:   set the thermal system into cooler mode.\n");
    printf("    monitor: starts to monitor input sensors.\n");
    printf("    fans:    [enable|disable] enables or disables fans.\n");
    printf("    peltier: [enable|disable] enables or disables peltier.\n");

    return 0;

}

int main (int argc, char *argv[]) {

    int enable = 0;
    int subcommand = parse_args(argc, argv, &enable);
    int sockfd;

    if (subcommand == SUB_CMD_UNKNOWN) {
        print_help();
        return 1;
    }

    if (utils_get_emcu_ip(ip_addr, sizeof(ip_addr))) {
        printf("Seems like emcu is not connected, exiting.\n");
        return 1;
    }

    sockfd = connect_to_server();
    if (sockfd < 0) {
        return 1;
    } else {
        serverfd = sockfd;
    }

    return emcu(subcommand, enable);

}
