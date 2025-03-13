/*
 * relay board driver
 * */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "ets_sys.h"

#include "driver/gpio.h"

#include "driver.h"
#include "pins.h"
#include "rbd.h"

static portMUX_TYPE muxtype = portMUX_INITIALIZER_UNLOCKED;
#define PORT_ENTER_CRITICAL portENTER_CRITICAL(&muxtype)
#define PORT_EXIT_CRITICAL portEXIT_CRITICAL(&muxtype)

/*#define CHECK(x) do { esp_err_t __; if ((__ = x) != ESP_OK) return __; } while (0)*/

static SemaphoreHandle_t rbd_mutex;

static uint16_t rbd_data = 0;

static const char *TAG = "rbd driver";

esp_err_t drv_rbd_push_data(uint16_t data) {

    uint16_t i = 0x8000;

    PORT_ENTER_CRITICAL;
    while (i) {
        gpio_set_level(RBD_DATA_PIN, !!(data & i));
        ets_delay_us(1);
        gpio_set_level(RBD_CLK_PIN, 0);
        ets_delay_us(1);
        gpio_set_level(RBD_CLK_PIN, 1);
        i = i >> 1;
    }
    ets_delay_us(1);
    PORT_EXIT_CRITICAL;

    rbd_data = data;

    return ESP_OK;
}

uint16_t drv_rbd_get_data(void) {
    return rbd_data;
}

esp_err_t drv_rbd_take_access(TickType_t wait_ticks) {

    int status;

    status = xSemaphoreTake(rbd_mutex, wait_ticks);

    if (status == pdTRUE) {
        return ESP_OK;
    }

    return ESP_FAIL;
}

esp_err_t drv_rbd_give_access(void) {

    int status;

    status = xSemaphoreGive(rbd_mutex);

    if (status == pdTRUE) {
        return ESP_OK;
    }

    return ESP_FAIL;
}

esp_err_t drv_rbd_init(void) {

    rbd_mutex = xSemaphoreCreateMutex();

    gpio_reset_pin(RBD_CLK_PIN);
    gpio_set_direction(RBD_CLK_PIN, GPIO_MODE_OUTPUT);
    /*
     * setting clk pins to high, because 74LS95 is falling edge triggered.
     * */
    gpio_set_level(RBD_CLK_PIN, 1);

    gpio_reset_pin(RBD_DATA_PIN);
    gpio_set_direction(RBD_DATA_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(RBD_DATA_PIN, 0);

    vTaskDelay(500 / portTICK_PERIOD_MS);

    ESP_LOGI(TAG, "pushing 0x00, to the rbd");
    drv_rbd_push_data(0);

    return ESP_OK;
}
