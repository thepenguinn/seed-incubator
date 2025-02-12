#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#include "utils.h"

int utils_get_emcu_ip(const char *mac_addr, char *ip_addr_dest) {

    int pid;
    int pipe_out[2];

    char buf[102];

    /*char *cmd[] = {*/
    /*    "./get_emcu_ip.sh",*/
    /*    MAC_ADDR,*/
    /*    NULL,*/
    /*};*/

    char *cmd[] = {
        "ip",
        "neigh",
        NULL,
    };

    pipe(pipe_out);

    pid = fork();

    if (pid < 0) {
        printf("fork() failed.\n");
        return 1;
    }

    if (pid == 0) {
        dup2(pipe_out[1], STDOUT_FILENO);
        write(pipe_out[1], "", sizeof(""));
        execvp(cmd[0], cmd);
        _exit(0);
    }

    int len = read(pipe_out[0], buf, sizeof(buf));

    if (len == 1) {
        return 1;
    }

    buf[len] = '\0';
    printf("%s", buf);

    wait(NULL);

    return 0;

}
