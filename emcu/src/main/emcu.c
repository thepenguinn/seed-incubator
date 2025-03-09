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

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    ESP_LOGI(MAIN_TAG, "leaving app_main");

}
