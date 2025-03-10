#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "wifi.h"
#include "tcp_server.h"
#include "emcu.h"

#include "driver/mux.h"
#include "driver/ldr.h"
#include "driver/sms.h"

/* TESTING */
#include "driver/test.h"

static const char *MAIN_TAG = "emcu main";

void app_main(void) {
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_start();
    wifi_init_sta_mode();

    /*drv_mux_init();*/
    tcp_server_init();

    drv_mux_init();

    /* TESTING */
    test_init();

    drv_ldr_init();
    drv_sms_init();
    int v[SMS_END];

    int i;
    while (1) {
        for (i = 0; i < SMS_END; i++) {
            v[i] = drv_sms_get_value(i, portMAX_DELAY);
        }
        /*ESP_LOGI(MAIN_TAG, "LDR_0: %d mV, LDR_1: %d mV, LDR_2: %d mV", v[0], v[1], v[2]);*/
        ESP_LOGI(MAIN_TAG, "SMS_0: %d mV, SMS_1: %d mV, SMS_2: %d mV, SMS_3: %d mV", v[0], v[1], v[2], v[3]);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    ESP_LOGI(MAIN_TAG, "leaving app_main");

}
