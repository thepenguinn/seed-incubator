#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

#include "emcu.h"
#include "emcu_test_server.h"

#include "esp_err.h"
#include "esp_log.h"


static const char *TAG = "test server";

static int serve_client(int client_sock);
static int accept_connection(int server_sock);
static esp_err_t receive_packet(int client_sock, char *buf);

int init_tcp_server(void);

static int serve_client(int client_sock) {

    /*
     * every request will be 5 bytes,
     *
     * first byte will be the request command.
     *
     * next four bytes will be data, the interpretation of this
     * data will depend on the command.
     *
     * the data will be uint32_t
     * it should be send in little endian order
     *
     * response will be differently sized.
     * monitor
     * */

    char buf[5];
    int len;
    esp_err_t ret;

    uint8_t cmd;
    uint32_t data;

    while (1) {

        ret = receive_packet(client_sock, buf);

        if (ret != ESP_OK) {
            break;
        }

        /*
         * data is in little endian
         * */

        cmd = (uint8_t) buf[0];

        data = (uint32_t) buf[1]
            | (((uint32_t) buf[2]) << 8)
            | (((uint32_t) buf[3]) << 16)
            | (((uint32_t) buf[4]) << 24);

        ESP_LOGW(TAG, "Got a packet, cmd: %d data: %d", (int) cmd, (int) data);
        /*
         * matching commands and calling corresponding handler functions
         * */

        /*
         * TODO: create a bit more sophisticated protocol, like SLIP used by the
         * UART <-> USB bridge of ESP.
         * */

        /*switch (cmd) {*/
        /*    case SUB_CMD_MONITOR:*/
        /*        sub_cmd_monitor_handler();*/
        /*        break;*/
        /*    case SUB_CMD_MUX:*/
        /*        sub_cmd_mux_push((uint8_t) data);*/
        /*        break;*/
        /*    case SUB_CMD_RBD:*/
        /*        sub_cmd_rbd_push((uint16_t)data);*/
        /*        break;*/
        /*    case SUB_CMD_MONITOR_TEMP:*/
        /*        send_uint32_t((uint32_t) drv_dht_get_value(DHT_0, DHT_DATA_TEMP, portMAX_DELAY));*/
        /*        send_uint32_t((uint32_t) drv_dht_get_value(DHT_1, DHT_DATA_TEMP, portMAX_DELAY));*/
        /*        break;*/
        /*    case SUB_CMD_MONITOR_HUME:*/
        /*        send_uint32_t((uint32_t) drv_dht_get_value(DHT_0, DHT_DATA_HUME, portMAX_DELAY));*/
        /*        send_uint32_t((uint32_t) drv_dht_get_value(DHT_1, DHT_DATA_HUME, portMAX_DELAY));*/
        /*        break;*/
        /*    case SUB_CMD_MONITOR_LDR:*/
        /*        send_uint32_t((uint32_t) drv_ldr_get_value(LDR_0, portMAX_DELAY));*/
        /*        send_uint32_t((uint32_t) drv_ldr_get_value(LDR_1, portMAX_DELAY));*/
        /*        send_uint32_t((uint32_t) drv_ldr_get_value(LDR_2, portMAX_DELAY));*/
        /*        break;*/
        /*    case SUB_CMD_MONITOR_SMS:*/
        /*        send_uint32_t((uint32_t) drv_sms_get_value(SMS_0, portMAX_DELAY));*/
        /*        send_uint32_t((uint32_t) drv_sms_get_value(SMS_1, portMAX_DELAY));*/
        /*        send_uint32_t((uint32_t) drv_sms_get_value(SMS_2, portMAX_DELAY));*/
        /*        send_uint32_t((uint32_t) drv_sms_get_value(SMS_3, portMAX_DELAY));*/
        /*        break;*/
        /*    case SUB_CMD_MONITOR_USO:*/
        /*        send_uint32_t((uint32_t) drv_uso_get_value(USO_0, portMAX_DELAY));*/
        /*        send_uint32_t((uint32_t) drv_uso_get_value(USO_1, portMAX_DELAY));*/
        /*        break;*/
        /*    default:*/
        /*        ESP_LOGW(TAG, "Got an invalid packet with cmd: %d", cmd);*/
        /*}*/

    }

    return 0;

}

static int accept_connection(int server_sock) {

    struct sockaddr_in conn_addr;
    unsigned int len = sizeof(conn_addr);
    int client_sock;

    while (1) {
        client_sock = accept(server_sock, (struct sockaddr *) &conn_addr, &len);

        if (client_sock < 0) {
            printf("accpet() failed.\n");
            return -1;
        }

        printf("GOT A CONNECTION.\n");
        serve_client(client_sock);
        close(client_sock);

    }

}

static esp_err_t receive_packet(int client_sock, char *buf) {

    /*
     * the buf should be atlest the size of 5 chars
     * */

    int i, len;

    for (i = 0; i < 5; i++) {

        len = recv(client_sock, buf + i, 1, 0);
        if (len < 0) {
            ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
            return ESP_FAIL;
        } else if (len == 0) {
            ESP_LOGW(TAG, "Connection closed");
            return ESP_FAIL;
        }

    }

    return ESP_OK;

}

int init_tcp_server(void) {

    int server_sock, connfd;
    struct sockaddr_in server_addr;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);

    if (server_sock == -1) {
        printf("socket() failed.\n");
        return -1;
    }
    printf("Created socket.\n");

    int enable = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        printf("setsockopt() failed.\n");
    }
    printf("Setted socket to reuseadder.\n");

    bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(TEST_IP_ADDR);
    server_addr.sin_port = (uint16_t) htons(PORT);

    if (bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        printf("bind() failed.\n");
        return -1;
    }
    printf("Binded successfully.\n");

    if (listen(server_sock, BACKLOG) == -1) {
        printf("listen() failed.\n");
        return -1;
    }

    return server_sock;

}

int main (int argc, char *argv[]) {

    int server_sock = init_tcp_server();

    if (server_sock <= 0) {
        printf("couldn't initalize server, exiting\n");
        return -1;
    }

    accept_connection(server_sock);

    return 0;

}
