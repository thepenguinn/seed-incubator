#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include <lwip/netdb.h>

#include "wifi.h"
#include "tcp_server.h"

static const char *TCP_TAG = "tcp server";

static int serve_client(int client_sock);
static void tcp_server_task(void *pvParameters);

static int send_all(int client_sock, char *data, size_t len) {

    int ret;

    while (len > 0) {
        ret = send(client_sock, data, 1, 0);
        if (ret < 0) {
            ESP_LOGE(TCP_TAG, "Error occurred during receiving: errno %d", errno);
            return 1;
        } else if (ret == 0) {
            ESP_LOGW(TCP_TAG, "Connection closed");
            return 1;
        }
        data++;
        len--;
    }
    return 0;
}

static int serve_client(int client_sock) {

    /*
     * every command will be two bytes, response will be differently sized.
     * monitor
     * */

    char buf[2];
    int len;

    while (1) {
        len = recv(client_sock, buf, sizeof(buf) - 1, 0);
        if (len < 0) {
            ESP_LOGE(TCP_TAG, "Error occurred during receiving: errno %d", errno);
            break;
        } else if (len == 0) {
            ESP_LOGW(TCP_TAG, "Connection closed");
            break;
        }
        recv(client_sock, buf + 1, sizeof(buf) - 1, 0);

        if (len < 0) {
            ESP_LOGE(TCP_TAG, "Error occurred during receiving: errno %d", errno);
            break;
        } else if (len == 0) {
            ESP_LOGW(TCP_TAG, "Connection closed");
            break;
        } else {
            if ((int) buf[0] == SUB_CMD_MONITOR) {
                xSemaphoreTake(temp_hume_mutex, portMAX_DELAY);
                if (send_all(client_sock, temp_value, sizeof(temp_value))) {
                    xSemaphoreGive(temp_hume_mutex);
                    break;
                }
                if (send_all(client_sock, hume_value, sizeof(hume_value))) {
                    xSemaphoreGive(temp_hume_mutex);
                    break;
                }
                xSemaphoreGive(temp_hume_mutex);

                xSemaphoreTake(light_mutex, portMAX_DELAY);
                if (send_all(client_sock, light_value, sizeof(light_value))) {
                    xSemaphoreGive(light_mutex);
                    break;
                }
                xSemaphoreGive(light_mutex);
            }
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

        wifi_sta_ip_await();

        ESP_LOGI(TCP_TAG, "Starting tcp server");

        bzero(&server_addr, sizeof(server_addr));

        server_addr.sin_addr.s_addr = inet_addr(ip_addr); // address from event handler
        /*server_addr.sin_addr.s_addr = htonl(INADDR_ANY);*/
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);

        int listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (listen_sock < 0) {
            ESP_LOGE(TCP_TAG, "Unable to create socket: errno %d", errno);
            continue;
        }
        int opt = 1;
        setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        int flags = fcntl(listen_sock, F_GETFL, 0);
        flags = flags | O_NONBLOCK;
        err = fcntl(listen_sock, F_SETFL, flags);
        if (err == -1) {
            ESP_LOGE(TCP_TAG, "Failed to set listening socket to NON BLOCKING, errno: %d", errno);
            goto CLEAN_UP;
        }

        ESP_LOGI(TCP_TAG, "Socket created");

        err = bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (err != 0) {
            ESP_LOGE(TCP_TAG, "Socket unable to bind: errno %d", errno);
            goto CLEAN_UP;
        }
        ESP_LOGI(TCP_TAG, "Socket bound, port %d", PORT);

        err = listen(listen_sock, 1);
        if (err != 0) {
            ESP_LOGE(TCP_TAG, "Error occurred during listen: errno %d", errno);
            goto CLEAN_UP;
        }

        ESP_LOGI(TCP_TAG, "Socket listening");

        /*
         * waiting for android
         * */

        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client_sock;

        for ( ;; ) {

            // keep polling
            for ( ;; ) {
                xSemaphoreTake(wifi_conn_mutex, portMAX_DELAY);
                if (wifi_connected == pdFALSE) {
                    // connection to wifi lost, goto CLEAN_UP and wait for wifi to connect
                    ESP_LOGW(TCP_TAG, "Seems like wifi has disconnected");
                    xSemaphoreGive(wifi_conn_mutex);
                    goto CLEAN_UP;
                }
                xSemaphoreGive(wifi_conn_mutex);
                // still connected, looking for connections.
                client_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &addr_len);
                if (client_sock < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                    ESP_LOGI(TCP_TAG, "waiting for 500ms before next poll");
                    vTaskDelay(500 / portTICK_PERIOD_MS);
                    continue;
                } else {
                    break;
                }
            }
            if (client_sock < 0) {
                // accept failed because of some other reason.
                ESP_LOGE(TCP_TAG, "Unable to accept connection: errno %d", errno);
                continue;
            }

            // Set tcp keepalive option
            setsockopt(client_sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
            setsockopt(client_sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
            setsockopt(client_sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
            setsockopt(client_sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));

            ESP_LOGI(TCP_TAG, "GOT ANDROID CONNECTION.");

            serve_client(client_sock);

            xSemaphoreTake(wifi_conn_mutex, portMAX_DELAY);
            if (wifi_connected == pdFALSE) {
                // connection to wifi lost, breaking from this loop
                ESP_LOGW(TCP_TAG, "Seems like wifi has disconnected");
                xSemaphoreGive(wifi_conn_mutex);
                break;
            }
            ESP_LOGW(TCP_TAG, "Connection to android ended, waiting for next");
            xSemaphoreGive(wifi_conn_mutex);

        }

    CLEAN_UP:
        close(listen_sock);

    } while (1);

}

esp_err_t tcp_server_init(void) {
    /*
     * TODO: intialize wifi
     * */
}
