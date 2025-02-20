#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "utils.h"
#include "emcu.h"

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
