#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "ets_sys.h"

#include "soc/soc_caps.h"
#include "driver/gpio.h"

#include "esp_adc_cal.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

/*#include "driver/ledc.h"*/

// maybe need this?
/*#include <stdio.h>*/
/*#include <stdlib.h>*/
/*#include <string.h>*/

#include "driver.h"
#include "pins.h"
#include "mux.h"

static portMUX_TYPE muxtype = portMUX_INITIALIZER_UNLOCKED;
#define PORT_ENTER_CRITICAL portENTER_CRITICAL(&muxtype)
#define PORT_EXIT_CRITICAL portEXIT_CRITICAL(&muxtype)

#define DIGITAL_READ 0
#define ANALOG_READ  1

/*#define CHECK(x) do { esp_err_t __; if ((__ = x) != ESP_OK) return __; } while (0)*/

static bool adc_calibration_init(
    adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle);
static void adc_calibration_deinit(adc_cali_handle_t handle);

static SemaphoreHandle_t mux_mutex;

static const char *TAG = "mux driver";

/*
 * ADC related
 * */
static adc_oneshot_unit_handle_t adc_handle;
static adc_cali_handle_t adc_cali_chan_handle = NULL;
static bool adc_cali_enable = false;
static int read_pin_mode = DIGITAL_READ;

/* proabably don't need this? */
/*static int read_pin_mode = DIGITAL_READ;*/

static bool adc_calibration_init(
    adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle) {

    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

    if (!calibrated) {
        /*ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");*/
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }

    *out_handle = handle;

    /*if (ret == ESP_OK) {*/
    /*    ESP_LOGI(TAG, "Calibration Success");*/
    /*}*/

    if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}

static void adc_calibration_deinit(adc_cali_handle_t handle) {

    ESP_LOGI(TAG, "deregister %s calibration scheme", "Line Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(handle));
    adc_cali_enable = false;
}

static esp_err_t analog_read_init(void) {

    //-------------ADC1 Init---------------//
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    //-------------ADC1 Config---------------//
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, MUX_ADC_CHANNEL, &config));

    //-------------ADC1 Calibration Init---------------//
    adc_cali_enable = adc_calibration_init(
        ADC_UNIT_1,
        MUX_ADC_CHANNEL,
        ADC_ATTEN_DB_12,
        &adc_cali_chan_handle
    );

    read_pin_mode = ANALOG_READ;

    return ESP_OK;
}

static esp_err_t analog_read_deinit(void) {

    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc_handle));
    if (adc_cali_enable) {
        adc_calibration_deinit(adc_cali_chan_handle);
    }

    return ESP_OK;

}

static esp_err_t digital_read_init(void) {

    gpio_reset_pin(MUX_READ_PIN);
    gpio_set_direction(MUX_READ_PIN, GPIO_MODE_INPUT);
    read_pin_mode = DIGITAL_READ;

    return ESP_OK;
}

static esp_err_t digital_read_deinit(void) {

    gpio_reset_pin(MUX_READ_PIN);
    return ESP_OK;
}

int drv_mux_read_analog(void) {

    int adc_raw;
    int voltage = -1;

    if (read_pin_mode != ANALOG_READ) {
        digital_read_deinit();
        analog_read_init();
    }

    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, MUX_ADC_CHANNEL, &adc_raw));
    if (adc_cali_enable) {
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc_cali_chan_handle, adc_raw, &voltage));
    }

    return voltage;

}

int drv_mux_read_digital(void) {

    if (read_pin_mode != DIGITAL_READ) {
        analog_read_deinit();
        digital_read_init();
    }

    return gpio_get_level(MUX_READ_PIN);
}

esp_err_t drv_mux_push_addr(uint8_t addr) {

    uint8_t i = 0x10;

    if (addr >= 32) {
        ESP_LOGW(TAG, "mux addr is greater than 31, upper bits will be ignored");
    }

    PORT_ENTER_CRITICAL;
    /*
     * disabling output before changing the address.
     * */
    gpio_set_level(MUX_EN_PIN, 0);
    ets_delay_us(10);
    while (i) {
        gpio_set_level(MUX_DATA_PIN, !(addr & i));
        ets_delay_us(1);
        gpio_set_level(MUX_CLK_PIN, 1);
        ets_delay_us(1);
        gpio_set_level(MUX_CLK_PIN, 0);
        i = i >> 1;
    }
    /*
     * enabling output back.
     * */
    ets_delay_us(1);
    gpio_set_level(MUX_EN_PIN, 1);
    ets_delay_us(1);
    PORT_EXIT_CRITICAL;

    return ESP_OK;
}

esp_err_t drv_mux_take_access(TickType_t wait_ticks) {

    int status;

    status = xSemaphoreTake(mux_mutex, wait_ticks);

    if (status == pdTRUE) {
        return ESP_OK;
    }

    return ESP_FAIL;
}

esp_err_t drv_mux_give_access(void) {

    int status;

    status = xSemaphoreGive(mux_mutex);

    if (status == pdTRUE) {
        return ESP_OK;
    }

    return ESP_FAIL;
}

esp_err_t drv_mux_init(void) {

    mux_mutex = xSemaphoreCreateMutex();

    gpio_reset_pin(MUX_CLK_PIN);
    gpio_reset_pin(MUX_DATA_PIN);
    gpio_reset_pin(MUX_EN_PIN);

    gpio_set_direction(MUX_CLK_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(MUX_DATA_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(MUX_EN_PIN, GPIO_MODE_OUTPUT);

    gpio_set_level(MUX_DATA_PIN, 0);
    gpio_set_level(MUX_CLK_PIN, 0);
    /*
     * disables output
     * */
    gpio_set_level(MUX_EN_PIN, 0);

    digital_read_init();

    analog_read_init();

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    ESP_LOGI(TAG, "pushing 0b00000, to the mux");
    drv_mux_push_addr(0);

    return ESP_OK;
}
