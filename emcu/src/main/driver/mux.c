#include "freertos/FreeRTOS.h"
/*#include "freertos/task.h"*/
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"
#include "ets_sys.h"

#include "driver/ledc.h"

#include "driver.h"
#include "mux.h"

static portMUX_TYPE muxtype = portMUX_INITIALIZER_UNLOCKED;
#define PORT_ENTER_CRITICAL portENTER_CRITICAL(&muxtype)
#define PORT_EXIT_CRITICAL portEXIT_CRITICAL(&muxtype)

#define CHECK(x) do { esp_err_t __; if ((__ = x) != ESP_OK) return __; } while (0)

static const char *TAG = "mux driver";

esp_err_t drv_mux_push_addr(uint8_t addr) {

    uint8_t i = 0x10;

    if (addr >= 32) {
        ESP_LOGW(TAG, "mux addr is greater than 31");
    }

    PORT_ENTER_CRITICAL;
    while (i) {
        gpio_set_level(DRV_MUX_DATA_GPIO, !(addr & i));
        ets_delay_us(1);
        gpio_set_level(DRV_MUX_CLK_GPIO, 1);
        ets_delay_us(1);
        gpio_set_level(DRV_MUX_CLK_GPIO, 0);
        i = i >> 1;
    }
    PORT_EXIT_CRITICAL;

    return ESP_OK;
}

esp_err_t drv_mux_init(void) {

    gpio_reset_pin(DRV_MUX_CLK_GPIO);
    gpio_reset_pin(DRV_MUX_DATA_GPIO);
    gpio_reset_pin(DRV_MUX_EN_GPIO);

    gpio_set_direction(DRV_MUX_CLK_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(DRV_MUX_DATA_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(DRV_MUX_EN_GPIO, GPIO_MODE_OUTPUT);

    gpio_set_level(DRV_MUX_DATA_GPIO, 0);
    gpio_set_level(DRV_MUX_CLK_GPIO, 0);
    gpio_set_level(DRV_MUX_EN_GPIO, 0); // disable output

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    ESP_LOGI(TAG, "pushing 0b00000, to the mux");
    drv_mux_push_addr(0);

    return ESP_OK;
}
