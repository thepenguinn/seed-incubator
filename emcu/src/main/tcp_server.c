#include "freertos/FreeRTOS.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include <lwip/netdb.h>

#include "esp_log.h"
#include "esp_err.h"

#include "wifi.h"
#include "tcp_server.h"

#include "driver/gpio.h"

#include "driver/mux.h"
#include "driver/rbd.h"
#include "driver/ldr.h"
#include "driver/sms.h"
#include "driver/dht.h"
#include "driver/ultrasonic.h"

#include "driver/pins.h"

static const char *TAG = "tcp server";

static int hstate = 0;

static esp_err_t send_all(int client_sock, char *data, size_t len);
static esp_err_t serve_client(int client_sock);

static esp_err_t sub_cmd_monitor_handler(void);

static void tcp_server_task(void *pvParameters);

static esp_err_t send_all(int client_sock, char *data, size_t len) {

    int ret;

    while (len > 0) {
        ret = send(client_sock, data, 1, 0);
        if (ret < 0) {
            ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
            return ESP_FAIL;
        } else if (ret == 0) {
            ESP_LOGW(TAG, "Connection closed");
            return ESP_FAIL;
        }
        data++;
        len--;
    }

    return ESP_OK;
}

static esp_err_t sub_cmd_monitor_handler(void) {

    ESP_LOGW(TAG, "Monitor");
    return ESP_OK;
}

static esp_err_t sub_cmd_mux_push(uint8_t addr) {
    drv_mux_take_access(portMAX_DELAY);
    drv_mux_push_addr(addr);
    drv_mux_give_access();
    return ESP_OK;
}

static esp_err_t sub_cmd_rbd_push(uint16_t data) {
    drv_rbd_take_access(portMAX_DELAY);
    drv_rbd_push_data(data);
    drv_rbd_give_access();
    return ESP_OK;
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

static esp_err_t send_uint32_t(int client_sock, uint32_t data) {


    char buf;
    int i;

    /*
     * data should be send in little endian
     * */

    for (i = 0; i < 4; i++) {
        buf = (char) ((data >> i * 8) & 0xff);
        send(client_sock, &buf, 1, 0);
    }

    return ESP_OK;
}

static esp_err_t serve_client(int client_sock) {

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
        switch (cmd) {
            case SUB_CMD_MONITOR:
                sub_cmd_monitor_handler();
                break;
            case SUB_CMD_MUX:
                sub_cmd_mux_push((uint8_t) data);
                break;
            case SUB_CMD_RBD:
                sub_cmd_rbd_push((uint16_t)data);
                break;
            case SUB_CMD_MONITOR_TEMP:
                send_uint32_t(client_sock, (uint32_t) drv_dht_get_value(DHT_0, DHT_DATA_TEMP, portMAX_DELAY));
                send_uint32_t(client_sock, (uint32_t) drv_dht_get_value(DHT_1, DHT_DATA_TEMP, portMAX_DELAY));
                break;
            case SUB_CMD_MONITOR_HUME:
                send_uint32_t(client_sock, (uint32_t) drv_dht_get_value(DHT_0, DHT_DATA_HUME, portMAX_DELAY));
                send_uint32_t(client_sock, (uint32_t) drv_dht_get_value(DHT_1, DHT_DATA_HUME, portMAX_DELAY));
                break;
            case SUB_CMD_MONITOR_LDR:
                send_uint32_t(client_sock, (uint32_t) drv_ldr_get_value(LDR_0, portMAX_DELAY));
                send_uint32_t(client_sock, (uint32_t) drv_ldr_get_value(LDR_1, portMAX_DELAY));
                send_uint32_t(client_sock, (uint32_t) drv_ldr_get_value(LDR_2, portMAX_DELAY));
                break;
            case SUB_CMD_MONITOR_SMS:
                send_uint32_t(client_sock, (uint32_t) drv_sms_get_value(SMS_0, portMAX_DELAY));
                send_uint32_t(client_sock, (uint32_t) drv_sms_get_value(SMS_1, portMAX_DELAY));
                send_uint32_t(client_sock, (uint32_t) drv_sms_get_value(SMS_2, portMAX_DELAY));
                send_uint32_t(client_sock, (uint32_t) drv_sms_get_value(SMS_3, portMAX_DELAY));
                break;
            case SUB_CMD_MONITOR_USO:
                send_uint32_t(client_sock, (uint32_t) drv_uso_get_value(USO_0, portMAX_DELAY));
                send_uint32_t(client_sock, (uint32_t) drv_uso_get_value(USO_1, portMAX_DELAY));
                break;
            case SUB_CMD_HUME:
                if (hstate == 0) {
                    gpio_set_level(HUMIDIFIER_PIN, 0);
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                    gpio_set_level(HUMIDIFIER_PIN, 1);
                    hstate = 1;
                } else {
                    gpio_set_level(HUMIDIFIER_PIN, 0);
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                    gpio_set_level(HUMIDIFIER_PIN, 1);
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                    gpio_set_level(HUMIDIFIER_PIN, 0);
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                    gpio_set_level(HUMIDIFIER_PIN, 1);
                    hstate = 0;
                }
                break;

            case SUB_CMD_PELTIER:
                drv_rbd_take_access(portMAX_DELAY);
                drv_rbd_peltier(!!data);
                drv_rbd_give_access();
                break;
            case SUB_CMD_PANEL_0:
                drv_rbd_take_access(portMAX_DELAY);
                drv_rbd_panel_0(!!data);
                drv_rbd_give_access();
                break;
            case SUB_CMD_FAN_0:
                drv_rbd_take_access(portMAX_DELAY);
                drv_rbd_fan_0(!!data);
                drv_rbd_give_access();
                break;
            case SUB_CMD_FAN_1:
                /* this pin is always high for some reason */
                drv_rbd_take_access(portMAX_DELAY);
                drv_rbd_fan_1(!!data);
                drv_rbd_give_access();
                break;
            case SUB_CMD_LIGHT_0:
                gpio_set_level(LIGHT_0_PIN, !!data);
                break;
            case SUB_CMD_LIGHT_1:
                gpio_set_level(LIGHT_1_PIN, !!data);
                break;

            default:
                ESP_LOGW(TAG, "Got an invalid packet with cmd: %d", cmd);
        }

    }

    return 0;

}

static void tcp_server_task(void *pvParameters) {

    int keepAlive = 1;
    int keepIdle = KEEPALIVE_IDLE;
    int keepInterval = KEEPALIVE_INTERVAL;
    int keepCount = KEEPALIVE_COUNT;
    int err;
    struct sockaddr_in server_addr;

    do {

        wifi_sta_ip_await(portMAX_DELAY);

        ESP_LOGI(TAG, "Starting tcp server");

        bzero(&server_addr, sizeof(server_addr));

        server_addr.sin_addr.s_addr = inet_addr(wifi_get_ip_addr()); // address from event handler
        /*server_addr.sin_addr.s_addr = htonl(INADDR_ANY);*/
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);

        int listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (listen_sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            continue;
        }
        int opt = 1;
        setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        int flags = fcntl(listen_sock, F_GETFL, 0);
        flags = flags | O_NONBLOCK;
        err = fcntl(listen_sock, F_SETFL, flags);
        if (err == -1) {
            ESP_LOGE(TAG, "Failed to set listening socket to NON BLOCKING, errno: %d", errno);
            goto CLEAN_UP;
        }

        ESP_LOGI(TAG, "Socket created");

        err = bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (err != 0) {
            ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
            goto CLEAN_UP;
        }
        ESP_LOGI(TAG, "Socket bound, port %d", PORT);

        err = listen(listen_sock, 1);
        if (err != 0) {
            ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
            goto CLEAN_UP;
        }

        ESP_LOGI(TAG, "Socket listening");

        /*
         * waiting for android
         * */

        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client_sock;

        for ( ;; ) {

            // keep polling
            for ( ;; ) {
                if (wifi_is_sta_connected(portMAX_DELAY) == ESP_FAIL) {
                    // connection to wifi lost, goto CLEAN_UP and wait for wifi to connect
                    ESP_LOGW(TAG, "Seems like wifi has disconnected");
                    goto CLEAN_UP;
                }
                // still connected, looking for connections.
                client_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &addr_len);
                if (client_sock < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                    /*ESP_LOGI(TAG, "waiting for 500ms before next poll");*/
                    vTaskDelay(500 / portTICK_PERIOD_MS);
                } else {
                    break;
                }
            }
            if (client_sock < 0) {
                // accept failed because of some other reason.
                ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
                continue;
            }

            // Set tcp keepalive option
            setsockopt(client_sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
            setsockopt(client_sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
            setsockopt(client_sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
            setsockopt(client_sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));

            ESP_LOGI(TAG, "GOT CLIENT CONNECTION.");

            serve_client(client_sock);
            close(client_sock);

            if (wifi_is_sta_connected(portMAX_DELAY) == ESP_FAIL) {
                // connection to wifi lost, breaking from this loop
                ESP_LOGW(TAG, "Seems like wifi has disconnected");
                break;
            }
            ESP_LOGW(TAG, "Connection to client ended, waiting for the next connection");

        }

    CLEAN_UP:
        close(listen_sock);

    } while (1);

}

esp_err_t tcp_server_init(void) {

    gpio_reset_pin(HUMIDIFIER_PIN);
    gpio_reset_pin(LIGHT_0_PIN);
    gpio_reset_pin(LIGHT_1_PIN);

    gpio_set_direction(HUMIDIFIER_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LIGHT_0_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LIGHT_1_PIN, GPIO_MODE_OUTPUT);

    gpio_set_level(HUMIDIFIER_PIN, 1);
    gpio_set_level(LIGHT_0_PIN, 0);
    gpio_set_level(LIGHT_1_PIN, 0);

   xTaskCreate(tcp_server_task, "tcp server", 4096, NULL, 5, NULL);
    return ESP_OK;
}


esp_err_t tcp_server_deinit(void) {
    // delete tcp_server_task
    return ESP_OK;
}
