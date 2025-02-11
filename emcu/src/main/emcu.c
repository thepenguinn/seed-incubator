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

#include "emcu.h"

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t wifi_event_group;

/*static SemaphoreHandle_t reserv_mutex;*/
/*static SemaphoreHandle_t temp_hume_mutex;*/
/*static SemaphoreHandle_t light_mutex;*/
/*static SemaphoreHandle_t soil_moist_mutex;*/
/**/
/*static float temp_value = 0;*/
/*static float hume_value = 0;*/
/**/
/*static float light_value = 0;*/
/*static float soil_moist_value = 0;*/
/*static float reserv_value = 0;*/

esp_event_handler_instance_t instance_any_id;
esp_event_handler_instance_t instance_got_ip;

static char ip_addr[128];

static const char *WIFI_TAG = "wifi task";
static const char *TCP_TAG = "tcp server";

static int s_retry_num = 0;

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data);

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
        ESP_LOGI(WIFI_TAG, "connect to the AP failed");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        sprintf(ip_addr, IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(WIFI_TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void) {

    wifi_event_group = xEventGroupCreate();

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
        int client_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &addr_len);
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

    CLEAN_UP:
        close(listen_sock);

    } while (1);


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
}
