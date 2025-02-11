#include <stdio.h>
#include <string.h>
#include "emcu.h"

static const char *sub_cmd_str[SUB_CMD_END] = {
    [SUB_CMD_UNKNOWN] = "unknown",
    [SUB_CMD_HMODE]   = "hmode",
    [SUB_CMD_CMODE]   = "cmode",
    [SUB_CMD_MONITOR] = "monitor",
    [SUB_CMD_FANS]    = "fans",
    [SUB_CMD_PELTIER] = "peltier",
};

int emcu (int command, int state);
int parse_args (int argc, char *argv[], int *enable);
int print_help (void);

int emcu_exhaust(int mode);
int emcu_monitor();
int emcu_fans(int state);
int emcu_peltier(int state);

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

    printf("hello world\n");

    int enable = 0;
    int subcommand = parse_args(argc, argv, &enable);

    if (subcommand == SUB_CMD_UNKNOWN) {
        print_help();
        return 1;
    }

    return emcu(subcommand, enable);

}
