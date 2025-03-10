#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "ets_sys.h"

#include "mux.h"

#define ANALOG_ADDR  0b00000
#define DIGITAL_ADDR 0b00001

static const char *ATAG = "ANALOG";
static const char *DTAG = "DIGITAL";

static void analog_read_task(void * pvParameters);

static void digital_read_task(void * pvParameters);

static void analog_read_task(void * pvParameters) {

    int value = 0;

    while (1) {
        /*ESP_LOGI(ATAG, "Trying to access mux");*/
        drv_mux_take_access(portMAX_DELAY);
        /*ESP_LOGI(ATAG, "Got access to mux, now pushing address and reading the value");*/
        drv_mux_push_addr(ANALOG_ADDR);
        value = drv_mux_read_analog();
        ESP_LOGI(ATAG, "Analog value %d", value);
        /*ESP_LOGI(ATAG, "Giving back the access to mux");*/
        drv_mux_give_access();

        /*ESP_LOGI(ATAG, "Waiting for 1 second");*/
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

}

static void digital_read_task(void * pvParameters) {

    int value = 0;

    while (1) {
        /*ESP_LOGI(DTAG, "Trying to access mux");*/
        drv_mux_take_access(portMAX_DELAY);
        /*ESP_LOGI(DTAG, "Got access to mux, now pushing address and reading the value");*/
        drv_mux_push_addr(DIGITAL_ADDR);
        value = drv_mux_read_digital();
        ESP_LOGI(DTAG, "Digital value %d", value);
        /*ESP_LOGI(DTAG, "Giving back the access to mux");*/
        drv_mux_give_access();

        /*ESP_LOGI(DTAG, "Waiting for 1 second");*/
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

}

esp_err_t test_init(void) {
    xTaskCreate(analog_read_task, "analog test task", 4096, NULL, 5, NULL);
    xTaskCreate(digital_read_task, "digital test task", 4096, NULL, 5, NULL);
    return ESP_OK;
}
