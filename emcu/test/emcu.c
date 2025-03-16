#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

#include "utils.h"
#include "emcu.h"
#include "emcu_test_server.h"

#include "draw.h"

static const char *sub_cmd_str[SUB_CMD_END] = {
    [SUB_CMD_UNKNOWN]      = "unknown",
    [SUB_CMD_HMODE]        = "hmode",
    [SUB_CMD_CMODE]        = "cmode",
    [SUB_CMD_MONITOR]      = "monitor",
    [SUB_CMD_FANS]         = "fans",
    [SUB_CMD_PELTIER]      = "peltier",
    [SUB_CMD_MUX]          = "mux",
    [SUB_CMD_RBD]          = "rbd",
    [SUB_CMD_MONITOR_TEMP] = "mtemp",
    [SUB_CMD_MONITOR_HUME] = "mhume",
    [SUB_CMD_MONITOR_LDR]  = "mldr",
    [SUB_CMD_MONITOR_SMS]  = "msms",
    [SUB_CMD_MONITOR_USO]  = "muso",
};

static char ip_addr[128];

static int serverfd;

int emcu(int command, int value);
int parse_args(int argc, char *argv[], int *value);
int print_help(void);

int emcu_exhaust(int mode);
int emcu_monitor();
int emcu_fans(int state);
int emcu_peltier(int state);
int emcu_mux(uint32_t value);
int emcu_rbd(uint32_t value);

uint32_t recieve_uint32_t(int serverfd);

int connect_to_server();

uint32_t recieve_uint32_t(int serverfd) {

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

static int send_packet(uint8_t cmd, uint32_t data) {

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

int emcu_exhaust(int mode) {
    return 0;
}

int emcu_monitor() {

    /*
     * TODO: create a tui app
     * */
    return 1;

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

int emcu_mux(uint32_t value) {

    send_packet(SUB_CMD_MUX, value);
    return 0;
}

int emcu_rbd(uint32_t value) {

    send_packet(SUB_CMD_RBD, value);
    return 0;
}

int emcu (int command, int value) {

    printf("%d, %d \n", command, value);

    switch(command) {

        case SUB_CMD_HMODE:
            return emcu_exhaust(EXHAUST_HMODE);
        case SUB_CMD_CMODE:
            return emcu_exhaust(EXHAUST_CMODE);
        case SUB_CMD_MONITOR:
            return emcu_monitor();
        case SUB_CMD_FANS:
            return emcu_fans(value);
        case SUB_CMD_PELTIER:
            return emcu_peltier(value);
        case SUB_CMD_MUX:
            return emcu_mux(value);
        case SUB_CMD_RBD:
            return emcu_rbd(value);
        case SUB_CMD_MONITOR_TEMP:
            send_packet(SUB_CMD_MONITOR_TEMP, 0);
            printf("TEMP_0: %d TEMP_1: %d\n", (int)recieve_uint32_t(serverfd), (int)recieve_uint32_t(serverfd));
            break;
        case SUB_CMD_MONITOR_HUME:
            send_packet(SUB_CMD_MONITOR_HUME, 0);
            printf("HUME_0: %d HUME_1: %d\n", (int)recieve_uint32_t(serverfd), (int)recieve_uint32_t(serverfd));
            break;
        case SUB_CMD_MONITOR_LDR:
            break;
        case SUB_CMD_MONITOR_SMS:
            break;
        case SUB_CMD_MONITOR_USO:
            break;
        default:
            return 1;

    }

    return 0;

}

static int tworaisedto(int num) {
    int value = 1;

    while (num > 0) {
        value = value * 2;
        num--;
    }

    return value;
}

static int strbtonum(char *string) {
    char *i = string;
    int count = -1;
    int value = 0;

    /*
     * counting how many 1s and 0s are there
     * */
    while (i && *i != '\0') {
        if (*i == '0' || *i == '1') {
            count++;
            i++;
        } else if (*i == '_') {
            i++;
            continue;
        } else {
            printf("There's other characters than 0, 1, _");
            break;
        }
    }

    i = string;
    /*
     * converting to number
     * */
    while (i && *i != '\0') {
        if (*i == '0') {
            count--;
            i++;
        } else if (*i == '1') {
            value = value + tworaisedto(count);
            count--;
            i++;
        } else if (*i == '_') {
            i++;
            continue;
        } else {
            printf("There's other characters than 0, 1, _");
            break;
        }
    }

    return value;

}

int parse_args (int argc, char *argv[], int *value) {

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

    if (value) {

        switch (subcmdint) {
            case SUB_CMD_FANS:
            case SUB_CMD_PELTIER:
            case SUB_CMD_MUX:
            case SUB_CMD_RBD:
                if (argc >= 3) {
                    *value = strbtonum(argv[2]);
                } else {
                    return SUB_CMD_UNKNOWN;
                }
                break;
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

void *test_fn (void *param) {
    return NULL;
}

int main (int argc, char *argv[]) {

    int value = 0;
    int subcommand = parse_args(argc, argv, &value);
    int sockfd;

    printf("%d\n", value);

    if (subcommand == SUB_CMD_UNKNOWN) {
        /*
         * start ncurses mode
         * */
        draw_init_ncurses();
        print_help();
        return 1;
    }

#ifdef __TEST_BUILD__
    strncpy(ip_addr, TEST_IP_ADDR, sizeof(TEST_IP_ADDR));
#else
    if (utils_get_emcu_ip(ip_addr, sizeof(ip_addr))) {
        printf("Seems like emcu is not connected, exiting.\n");
       return 1;
    }
#endif

    sockfd = connect_to_server();
    if (sockfd < 0) {
        return 1;
    } else {
        serverfd = sockfd;
    }

    return emcu(subcommand, value);

}
