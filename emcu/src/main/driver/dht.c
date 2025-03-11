#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"

#include "driver/gpio.h"

#include "driver.h"
#include "pins.h"
#include "mux.h"

#include "lib/dht.h"
#include "dht.h"

static const char *TAG = "dht driver";

static SemaphoreHandle_t dht_mutex;

/*
 * the data will be in int, that means it needs to be divided
 * by zero to convert it into percentage or degree celcius
 * */
static int dht_values[DHT_END][DHT_DATA_END];

static void dht_task(void *pvParameters);

static void dht_task(void *pvParameters) {

    int dht_v[DHT_END][DHT_DATA_END];
    int i, status, ret;

    while (1) {
        drv_mux_take_access(portMAX_DELAY);
        for (i = 0; i < DHT_END; i++) {
            drv_mux_push_addr(MUX_ADDR_DHT_OFFSET + i);

            /*
             * this will set the pin to digital mode
             * */
            drv_mux_read_digital();

            gpio_set_direction(MUX_READ_PIN, GPIO_MODE_OUTPUT);
            gpio_set_level(MUX_READ_PIN, 1);
            vTaskDelay(200 / portTICK_PERIOD_MS);
            /* read using library */

            setDHTgpio(MUX_READ_PIN);
            ret = readDHT();

            switch (ret) {
                case DHT_TIMEOUT_ERROR:
                    ESP_LOGE(TAG, "Sensor Timeout\n");
                    break;

                case DHT_CHECKSUM_ERROR:
                    ESP_LOGE(TAG, "CheckSum error\n");
                    break;
                case DHT_OK:
                    ESP_LOGW(TAG, "Got data %d", i);
                    dht_v[i][DHT_DATA_TEMP] = getTemperature();
                    dht_v[i][DHT_DATA_HUME] = getHumidity();
                    break;
                default:
                    ESP_LOGE(TAG, "Unknown error\n");
            }

            /*
             * this will reset the pin to digital input mode
             * */
            gpio_reset_pin(MUX_READ_PIN);
            gpio_set_direction(MUX_READ_PIN, GPIO_MODE_INPUT);
        }
        drv_mux_give_access();

        status = xSemaphoreTake(dht_mutex, portMAX_DELAY);
        if (status == pdFALSE) {
            ESP_LOGW(TAG, "couldn't take dht_mutex");
        }
        for (i = 0; i < DHT_END; i++) {
            dht_values[i][DHT_DATA_TEMP] = dht_v[i][DHT_DATA_TEMP];
            dht_values[i][DHT_DATA_HUME] = dht_v[i][DHT_DATA_HUME];
        }
        status = xSemaphoreGive(dht_mutex);

        if (status == pdFALSE) {
            ESP_LOGW(TAG, "couldn't give dht_mutex");
        }

        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

int drv_dht_get_value(
    enum DhtNum dht_num, enum DhtData dht_data, TickType_t wait_ticks) {

    if (dht_num < 0 || dht_num >= DHT_END) {
        ESP_LOGW(TAG, "dht number out of range.");
        return -1;
    }

    if (dht_data < 0 || dht_data >= DHT_DATA_END) {
        ESP_LOGW(TAG, "dht data index out of range.");
        return ESP_FAIL;
    }

    int status = xSemaphoreTake(dht_mutex, wait_ticks);
    if (status == pdFALSE) {
        return -1;
    }
    int dht_val = dht_values[dht_num][dht_data];
    status = xSemaphoreGive(dht_mutex);

    if (status == pdFALSE) {
        return -1;
    }

    return dht_val;

}

esp_err_t drv_dht_set_value(
    enum DhtNum dht_num, enum DhtData dht_data, int value, TickType_t wait_ticks) {

    if (dht_num < 0 || dht_num >= DHT_END) {
        ESP_LOGW(TAG, "dht number index out of range.");
        return ESP_FAIL;
    }

    if (dht_data < 0 || dht_data >= DHT_DATA_END) {
        ESP_LOGW(TAG, "dht data index out of range.");
        return ESP_FAIL;
    }

    int status = xSemaphoreTake(dht_mutex, wait_ticks);
    if (status == pdFALSE) {
        return ESP_FAIL;
    }
    dht_values[dht_num][dht_data] = value;
    status = xSemaphoreGive(dht_mutex);

    if (status == pdFALSE) {
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t drv_dht_init(void) {

    dht_mutex = xSemaphoreCreateMutex();
    int i, status;

    status = xSemaphoreTake(dht_mutex, portMAX_DELAY);
    if (status == pdFALSE) {
        ESP_LOGW(TAG, "couldn't take dht_mutex");
    }
    for (i = 0; i < DHT_END; i++) {
        dht_values[i][DHT_DATA_TEMP] = 0;
        dht_values[i][DHT_DATA_HUME] = 0;
    }
    status = xSemaphoreGive(dht_mutex);

    if (status == pdFALSE) {
        ESP_LOGW(TAG, "couldn't give dht_mutex");
    }

    xTaskCreate(dht_task, "dht task", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "initialized dht sensors");
    return ESP_OK;
}

esp_err_t drv_dht_deinit(void) {
    /*
     * TODO:
     * */
    return ESP_OK;
}
