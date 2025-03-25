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
#include "tcp.h"

static char *sub_cmd_str[SUB_CMD_END] = {
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
    [SUB_CMD_TUI]          = "tui",
    [SUB_CMD_PANEL_0]      = "panel0",
    [SUB_CMD_HUME]         = "hume",
    [SUB_CMD_FAN_0]        = "fan0",
    [SUB_CMD_FAN_1]        = "fan1",
    [SUB_CMD_LIGHT_0]      = "light0",
    [SUB_CMD_LIGHT_1]      = "light1",
};

static int serverfd;

int emcu(int command, int value);
int parse_args(int argc, char *argv[], int *value);
int print_help(void);

int emcu_exhaust(int mode) {
    return 0;
}

int emcu_monitor() {
    return 0;
}

int emcu_fans(int state) {
    return 0;
}

int emcu_peltier(int state) {
    tcp_send_packet(SUB_CMD_HUME, !!state);
    return 0;
}

int emcu_mux(uint32_t value) {

    tcp_send_packet(SUB_CMD_MUX, value);
    return 0;
}

int emcu_rbd(uint32_t value) {

    tcp_send_packet(SUB_CMD_RBD, value);
    return 0;
}

int emcu_hume(int state) {
    tcp_send_packet(SUB_CMD_HUME, !!state);
    return 0;
}

int emcu_fan_0(int state) {
    tcp_send_packet(SUB_CMD_FAN_0, !!state);
    return 0;
}

int emcu_fan_1(int state) {
    tcp_send_packet(SUB_CMD_FAN_1, !!state);
    return 0;
}

int emcu_light_0(int state) {
    tcp_send_packet(SUB_CMD_LIGHT_0, !!state);
    return 0;
}

int emcu_light_1(int state) {
    tcp_send_packet(SUB_CMD_LIGHT_1, !!state);
    return 0;
}

int emcu_panel_0(int state) {
    tcp_send_packet(SUB_CMD_PANEL_0, !!state);
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
            tcp_send_packet(SUB_CMD_MONITOR_TEMP, 0);
            printf(
                "TEMP_0: %d TEMP_1: %d\n",
                (int)tcp_recieve_uint32_t(),
                (int)tcp_recieve_uint32_t()
            );
            break;
        case SUB_CMD_MONITOR_HUME:
            tcp_send_packet(SUB_CMD_MONITOR_HUME, 0);
            printf(
                "HUME_0: %d HUME_1: %d\n",
                (int)tcp_recieve_uint32_t(),
                (int)tcp_recieve_uint32_t()
            );
            break;
        case SUB_CMD_MONITOR_LDR:
            break;
        case SUB_CMD_MONITOR_SMS:
            break;
        case SUB_CMD_MONITOR_USO:
            break;
        case SUB_CMD_TUI:
            /*
             * start the tui app
             * */
            break;
        case SUB_CMD_HUME:
            emcu_panel_0(value);
            break;
        case SUB_CMD_HUME:
            emcu_hume(value);
            break;
        case SUB_CMD_FAN_0:
            emcu_fan_0(value);
            break;
        case SUB_CMD_FAN_1:
            emcu_fan_1(value);
            break;
        case SUB_CMD_LIGHT_0:
            emcu_light_0(value);
            break;
        case SUB_CMD_LIGHT_1:
            emcu_light_1(value);
            break;
        default:
            return 1;

    }

    return 0;

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
            case SUB_CMD_HUME:
            case SUB_CMD_FAN_0:
            case SUB_CMD_FAN_1:
            case SUB_CMD_LIGHT_0:
            case SUB_CMD_LIGHT_1:
                if (argc >= 3) {
                    *value = utils_strbtonum(argv[2]);
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

int main (int argc, char *argv[]) {

    int value = 0;
    int subcommand = parse_args(argc, argv, &value);
    int sockfd;

    printf("%d\n", value);

    if (subcommand == SUB_CMD_UNKNOWN) {
        print_help();
        return 1;
    }

    char *ip_addr = tcp_get_ip_addr_buf();

#ifdef __TEST_BUILD__
    strncpy(ip_addr, TEST_IP_ADDR, sizeof(TEST_IP_ADDR));
#else
    if (utils_get_emcu_ip(ip_addr, sizeof(ip_addr)) && subcommand != SUB_CMD_TUI) {
        printf("Seems like emcu is not connected, exiting.\n");
       return 1;
    }
#endif

    if (subcommand == SUB_CMD_TUI) {
        draw_init_ncurses();
        draw_start_tui_app();
        draw_deinit_ncurses();
        return 0;
    }

    sockfd = tcp_connect_to_server(ip_addr, PORT, 0);
    if (sockfd < 0) {
        printf("Failed to connected to emcu\n");
        return 1;
    }

    return emcu(subcommand, value);

}
