#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"

#include "driver.h"
#include "pins.h"
#include "mux.h"

#include "ldr.h"

static const char *TAG = "ldr driver";

static SemaphoreHandle_t ldr_mutex;

static int ldr_values[LDR_END];

static void ldr_task(void *pvParameters);

static void ldr_task(void *pvParameters) {

    int ldr_v[LDR_END];
    int i, status;

    while (1) {
        drv_mux_take_access(portMAX_DELAY);
        for (i = MUX_ADDR_LDR_OFFSET; i < LDR_END; i++) {
            drv_mux_push_addr(MUX_ADDR_LDR_OFFSET + i);
            ldr_v[i] = drv_mux_read_analog();
        }
        drv_mux_give_access();

        status = xSemaphoreTake(ldr_mutex, portMAX_DELAY);
        if (status == pdFALSE) {
            ESP_LOGW(TAG, "couldn't take ldr_mutex");
        }
        for (i = 0; i < LDR_END; i++) {
            ldr_values[i] = ldr_v[i];
        }
        status = xSemaphoreGive(ldr_mutex);

        if (status == pdFALSE) {
            ESP_LOGW(TAG, "couldn't give ldr_mutex");
        }

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

int drv_ldr_get_value(enum LdrNum ldr_num, TickType_t wait_ticks) {

    if (ldr_num < 0 || ldr_num >= LDR_END) {
        ESP_LOGW(TAG, "ldr number out of range.");
        return -1;
    }

    int status = xSemaphoreTake(ldr_mutex, wait_ticks);
    if (status == pdFALSE) {
        return -1;
    }
    int ldr_val = ldr_values[ldr_num];
    status = xSemaphoreGive(ldr_mutex);

    if (status == pdFALSE) {
        return -1;
    }

    return ldr_val;

}

esp_err_t drv_ldr_set_value(enum LdrNum ldr_num, int value, TickType_t wait_ticks) {

    if (ldr_num < 0 || ldr_num >= LDR_END) {
        ESP_LOGW(TAG, "ldr number out of range.");
        return ESP_FAIL;
    }

    int status = xSemaphoreTake(ldr_mutex, wait_ticks);
    if (status == pdFALSE) {
        return ESP_FAIL;
    }
    ldr_values[ldr_num] = value;
    status = xSemaphoreGive(ldr_mutex);

    if (status == pdFALSE) {
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t drv_ldr_init(void) {

    ldr_mutex = xSemaphoreCreateMutex();
    int i, status;

    status = xSemaphoreTake(ldr_mutex, portMAX_DELAY);
    if (status == pdFALSE) {
        ESP_LOGW(TAG, "couldn't take ldr_mutex");
    }
    for (i = 0; i < LDR_END; i++) {
        ldr_values[i] = 0;
    }
    status = xSemaphoreGive(ldr_mutex);

    if (status == pdFALSE) {
        ESP_LOGW(TAG, "couldn't give ldr_mutex");
    }

    xTaskCreate(ldr_task, "ldr task", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "initialized ldr sensors");
    return ESP_OK;
}

esp_err_t drv_ldr_deinit(void) {
    return ESP_OK;
}
