#include "freertos/FreeRTOS.h"
/*#include "freertos/task.h"*/
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"
#include "ets_sys.h"

#include "driver.h"
#include "pins.h"
#include "exhaust.h"

static portMUX_TYPE muxtype = portMUX_INITIALIZER_UNLOCKED;
#define PORT_ENTER_CRITICAL portENTER_CRITICAL(&muxtype)
#define PORT_EXIT_CRITICAL portEXIT_CRITICAL(&muxtype)

static const char *TAG = "exhaust driver";

esp_err_t drv_ext_push_data(uint8_t data) {

    uint8_t i = 0x80;

    PORT_ENTER_CRITICAL;
    while (i) {
        gpio_set_level(EXT_DATA_PIN, !!(data & i));
        ets_delay_us(1);
        /*
         * data is clk'ed in at the falling edge
         * */
        gpio_set_level(EXT_CLK_PIN, 1);
        ets_delay_us(1);
        gpio_set_level(EXT_CLK_PIN, 0);
        ets_delay_us(1);
        gpio_set_level(EXT_CLK_PIN, 1);
        i = i >> 1;
    }
    PORT_EXIT_CRITICAL;

    return ESP_OK;
}

esp_err_t drv_ext_init(void) {

    gpio_reset_pin(EXT_CLK_PIN);
    gpio_reset_pin(EXT_DATA_PIN);

    gpio_set_direction(EXT_CLK_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(EXT_DATA_PIN, GPIO_MODE_OUTPUT);

    /*
     * 74LS95 will clk in data at the falling edge of the clk,
     * there for setting the clk to high
     * */
    gpio_set_level(EXT_CLK_PIN, 1);
    gpio_set_level(EXT_DATA_PIN, 0);

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    ESP_LOGI(TAG, "pushing 0x00, to the exhaust control board");
    drv_ext_push_data(0);

    return ESP_OK;

}
