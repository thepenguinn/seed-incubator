#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "utils.h"
#include "emcu.h"

static int tworaisedto(int num) {
    int value = 1;

    while (num > 0) {
        value = value * 2;
        num--;
    }

    return value;
}

int utils_get_emcu_ip(char *ip_addr_dest, const size_t size) {

    char buf[128] = {};

    char *cmd = "ip -f inet -br neigh | grep " MAC_ADDR " | awk '{printf $1}'";
    FILE *fd = popen(cmd, "r");
    int ret;

    if (fd == NULL) {
        fprintf(stderr, "popen() failed, while retrieving ip of emcu\n");
        return 1;
    }

    size_t len = fread(buf, sizeof(char), sizeof(buf) - 1, fd);

    if (len == 0) {
        // emcu is not connected
        return 1;
    }

    ret = pclose(fd);

    if (ret < 0) {
        fprintf(stderr, "pclose() returned -1\n");
        return 1;
    }

    strncpy(ip_addr_dest, buf, len);

    return 0;

}

int utils_strbtonum(char *string) {
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
