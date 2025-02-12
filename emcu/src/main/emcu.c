/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include <lwip/netdb.h>

#include "driver/gpio.h"

#include "emcu.h"
#include "DHT.h"

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t wifi_event_group;

static SemaphoreHandle_t temp_hume_mutex;

/*static SemaphoreHandle_t reserv_mutex;*/
/*static SemaphoreHandle_t light_mutex;*/
/*static SemaphoreHandle_t soil_moist_mutex;*/

static char temp_value[6];
static char hume_value[6];

/*static float light_value = 0;*/
/*static float soil_moist_value = 0;*/
/*static float reserv_value = 0;*/

static SemaphoreHandle_t wifi_conn_mutex;
static int wifi_connected = pdFALSE;

esp_event_handler_instance_t instance_any_id;
esp_event_handler_instance_t instance_got_ip;

static char ip_addr[128];

static const char *WIFI_TAG = "wifi task";
static const char *TCP_TAG = "tcp server";
static const char *DHT_TAG = "dht task";

static int s_retry_num = 0;

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data);

static int serve_client(int client_sock);

void temp_hume_task(void *pvParameters);

void tcp_server_task(void *pvParameters);

void wifi_init_sta(void);

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data) {

    ESP_LOGI(WIFI_EVENT, "Got into event_handler");

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < WIFI_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(WIFI_TAG, "retry to connect to the AP");
        }

        xSemaphoreTake(wifi_conn_mutex, portMAX_DELAY);
        wifi_connected = pdFALSE;
        xSemaphoreGive(wifi_conn_mutex);

        ESP_LOGI(WIFI_TAG, "connect to the AP failed");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        sprintf(ip_addr, IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(WIFI_TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;

        xSemaphoreTake(wifi_conn_mutex, portMAX_DELAY);
        wifi_connected = pdTRUE;
        xSemaphoreGive(wifi_conn_mutex);

        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void) {

    wifi_event_group = xEventGroupCreate();
    wifi_conn_mutex = xSemaphoreCreateMutex();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /*
     * starting tcp task
     * */

    xTaskCreate(tcp_server_task, "tcp server", 4096, NULL, 5, NULL);

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &event_handler,
            NULL,
            &instance_any_id));

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &event_handler,
            NULL,
            &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
            .sae_pwe_h2e = ESP_WIFI_SAE_MODE,
            .sae_h2e_identifier = EXAMPLE_H2E_IDENTIFIER,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(WIFI_TAG, "wifi_init_sta finished.");

}

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
                    break;
                }
                if (send_all(client_sock, hume_value, sizeof(hume_value))) {
                    break;
                }
                xSemaphoreGive(temp_hume_mutex);
            }
        }
    }

    return 0;

}

void tcp_server_task(void *pvParameters) {

    EventBits_t bits;
    int keepAlive = 1;
    int keepIdle = KEEPALIVE_IDLE;
    int keepInterval = KEEPALIVE_INTERVAL;
    int keepCount = KEEPALIVE_COUNT;
    struct sockaddr_in server_addr;

    do {

        ESP_LOGI(TCP_TAG, "Waiting for wifi to connect");
        bits = xEventGroupWaitBits(
            wifi_event_group,
            WIFI_CONNECTED_BIT,
            pdTRUE,
            pdFALSE,
            portMAX_DELAY);

        if (!(bits & WIFI_CONNECTED_BIT)) {
            ESP_LOGE(TCP_TAG, "UNEXPECTED EVENT");
            continue;
        }

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

        ESP_LOGI(TCP_TAG, "Socket created");

        int err = bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
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

            client_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &addr_len);
            if (client_sock < 0) {
                ESP_LOGE(TCP_TAG, "Unable to accept connection: errno %d", errno);
                break;
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

void temp_hume_task(void *pvParameters) {

    setDHTgpio(GPIO_NUM_4);
    ESP_LOGI(DHT_TAG, "Starting DHT Task\n\n");

    for ( ;; ) {

        ESP_LOGI(DHT_TAG, "=== Reading DHT ===\n");
        int ret = readDHT();

        errorHandler(ret);

        ESP_LOGI(DHT_TAG, "Hum: %.1f Tmp: %.1f\n", getHumidity(), getTemperature());

        xSemaphoreTake(temp_hume_mutex, portMAX_DELAY);
        snprintf(temp_value, sizeof(temp_value) - 1, "%.1f", getTemperature());
        snprintf(hume_value, sizeof(hume_value) - 1, "%.1f", getHumidity());
        xSemaphoreGive(temp_hume_mutex);

        vTaskDelay(3000 / portTICK_PERIOD_MS);

    }

}

void app_main(void) {
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(WIFI_TAG, "ESP_WIFI_MODE_STA");

    wifi_init_sta();

    esp_rom_gpio_pad_select_gpio(GPIO_NUM_4);

    temp_hume_mutex = xSemaphoreCreateMutex();

    xTaskCreate(temp_hume_task, "temp_hume_task", 4096, NULL, 5, NULL);

}
