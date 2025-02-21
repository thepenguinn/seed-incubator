#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_event.h"

#include "esp_log.h"
#include "esp_err.h"

#include "wifi.h"

static const char *WIFI_TAG = "wifi task";

/*
 * for sta, this event will be triggered after esp gets the ip
 * after connecting to specified access point.
 * */
static EventGroupHandle_t wifi_sta_ip_event_group;

static void wifi_sta_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data);

static void wifi_sta_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data) {

    ESP_LOGI(WIFI_TAG, "ev base: %s, ev id: %d", event_base, (int)event_id);
    ESP_LOGI(WIFI_TAG, "Got into event_handler");

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < WIFI_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(WIFI_TAG, "retry to connect to the AP");
        }

        /*xSemaphoreTake(wifi_conn_mutex, portMAX_DELAY);*/
        /*wifi_connected = pdFALSE;*/
        /*xSemaphoreGive(wifi_conn_mutex);*/

        ESP_LOGI(WIFI_TAG, "connect to the AP failed");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        sprintf(ip_addr, IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(WIFI_TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;

        /*xSemaphoreTake(wifi_conn_mutex, portMAX_DELAY);*/
        /*wifi_connected = pdTRUE;*/
        /*xSemaphoreGive(wifi_conn_mutex);*/

        xEventGroupSetBits(wifi_sta_ip_event_group, WIFI_CONNECTED_BIT);
    }
}


esp_err_t wifi_sta_ip_await(TickType_t wait_ticks) {

    EventBits_t bits;

    bits = xEventGroupWaitBits(
        wifi_sta_ip_event_group,
        WIFI_STA_IP_BIT,
        pdTRUE,
        pdFALSE,
        wait_ticks);

    if (!(bits & WIFI_STA_IP_BIT)) {
        ESP_LOGE(WIFI_TAG, "UNEXPECTED EVENT");
        return ESP_FAIL;
    }

    return ESP_OK;

}

esp_err_t wifi_init_sta_mode(void) {

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    wifi_sta_ip_event_group = xEventGroupCreate();
    /*wifi_conn_mutex = xSemaphoreCreateMutex();*/

    /*
     * TODO: figure out what ESP_ERROR_CHECK is doing, and properly
     * return ESP_FAIL, if needed.
     * */

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /*
     * starting tcp task
     * TODO: well this should go
     * */

    /*xTaskCreate(tcp_server_task, "tcp server", 4096, NULL, 5, NULL);*/

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &wifi_sta_event_handler,
            NULL,
            &instance_any_id));

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &wifi_sta_event_handler,
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

    ESP_LOGI(WIFI_TAG, "wifi_init_sta_mode finished.");

    return ESP_OK;

}
