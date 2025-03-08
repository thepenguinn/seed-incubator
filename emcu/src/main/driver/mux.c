#include "freertos/FreeRTOS.h"
/*#include "freertos/task.h"*/
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"
#include "ets_sys.h"

#include "driver/ledc.h"

#include "mux.h"

static portMUX_TYPE muxtype = portMUX_INITIALIZER_UNLOCKED;
#define PORT_ENTER_CRITICAL portENTER_CRITICAL(&muxtype)
#define PORT_EXIT_CRITICAL portEXIT_CRITICAL(&muxtype)

#define CHECK(x) do { esp_err_t __; if ((__ = x) != ESP_OK) return __; } while (0)

/*#define LEDC_TIMER              LEDC_TIMER_0*/
/*#define LEDC_MODE               LEDC_LOW_SPEED_MODE*/
/*#define LEDC_OUTPUT_IO          (4) // Define the output GPIO*/
/*#define LEDC_CHANNEL            LEDC_CHANNEL_0*/
/*#define LEDC_DUTY_RES           LEDC_TIMER_4_BIT // Set duty resolution to 13 bits*/
/*#define LEDC_DUTY               (4096) // Set duty to 50%. (2 ** 13) * 50% = 4096*/
/*#define LEDC_FREQUENCY          (1700000) // Frequency in Hertz. Set frequency at 4 kHz*/

/* Warning:
 * For ESP32, ESP32S2, ESP32S3, ESP32C3, ESP32C2, ESP32C6, ESP32H2, ESP32P4 targets,
 * when LEDC_DUTY_RES selects the maximum duty resolution (i.e. value equal to SOC_LEDC_TIMER_BIT_WIDTH),
 * 100% duty cycle is not reachable (duty cannot be set to (2 ** SOC_LEDC_TIMER_BIT_WIDTH)).
 */

/*static void example_ledc_init(void)*/
/*{*/
/*    // Prepare and then apply the LEDC PWM timer configuration*/
/*    ledc_timer_config_t ledc_timer = {*/
/*        .speed_mode       = LEDC_MODE,*/
/*        .duty_resolution  = LEDC_DUTY_RES,*/
/*        .timer_num        = LEDC_TIMER,*/
/*        .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 4 kHz*/
/*        .clk_cfg          = LEDC_AUTO_CLK*/
/*    };*/
/*    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));*/
/**/
/*    // Prepare and then apply the LEDC PWM channel configuration*/
/*    ledc_channel_config_t ledc_channel = {*/
/*        .speed_mode     = LEDC_MODE,*/
/*        .channel        = LEDC_CHANNEL,*/
/*        .timer_sel      = LEDC_TIMER,*/
/*        .intr_type      = LEDC_INTR_DISABLE,*/
/*        .gpio_num       = LEDC_OUTPUT_IO,*/
/*        .duty           = 0, // Set duty to 0%*/
/*        .hpoint         = 0*/
/*    };*/
/*    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));*/
/*}*/

static esp_err_t drv_mux_push_addr(int clk_pin, int data_pin, uint16_t addr);

static esp_err_t drv_mux_push_addr(int clk_pin, int data_pin, uint16_t addr) {

    uint16_t i = 0x8000;

    PORT_ENTER_CRITICAL;
    while (i) {
        gpio_set_level(data_pin, !!(addr & i));
        ets_delay_us(1);
        gpio_set_level(clk_pin, 0);
        ets_delay_us(1);
        gpio_set_level(clk_pin, 1);
        i = i >> 1;
    }
    PORT_EXIT_CRITICAL;

    return ESP_OK;
}

esp_err_t drv_mux_init(void) {

    gpio_reset_pin(DRV_MUX_CLK_GPIO); // 4
    gpio_reset_pin(DRV_MUX_DATA_GPIO); // 5

    gpio_set_direction(DRV_MUX_CLK_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(DRV_MUX_DATA_GPIO, GPIO_MODE_OUTPUT);

    gpio_set_level(DRV_MUX_DATA_GPIO, DRV_GPIO_LOW);
    gpio_set_level(DRV_MUX_CLK_GPIO, DRV_GPIO_HIGH);

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    uint16_t i;

    while (1) {
        i = 0x8000;
        while (i) {
            drv_mux_push_addr(DRV_MUX_CLK_GPIO, DRV_MUX_DATA_GPIO, i);
            i = i >> 1;
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
    }

    /*// Set the LEDC peripheral configuration*/
    /*example_ledc_init();*/
    /*// Set duty to 50%*/
    /*ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY));*/
    /*// Update duty to apply the new value*/
    /*ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));*/

    return ESP_OK;
}
