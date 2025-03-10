#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"

#include "driver.h"
#include "pins.h"
#include "mux.h"

#include "sms.h"

static const char *TAG = "sms driver";

static SemaphoreHandle_t sms_mutex;

static int sms_values[SMS_END];

static void sms_task(void *pvParameters);

static void sms_task(void *pvParameters) {

    int sms_v[SMS_END];
    int i, status;

    while (1) {
        drv_mux_take_access(portMAX_DELAY);
        for (i = 0; i < SMS_END; i++) {
            drv_mux_push_addr(MUX_ADDR_SMS_OFFSET + i);
            sms_v[i] = drv_mux_read_analog();
        }
        drv_mux_give_access();

        status = xSemaphoreTake(sms_mutex, portMAX_DELAY);
        if (status == pdFALSE) {
            ESP_LOGW(TAG, "couldn't take sms_mutex");
        }
        for (i = 0; i < SMS_END; i++) {
            sms_values[i] = sms_v[i];
        }
        status = xSemaphoreGive(sms_mutex);

        if (status == pdFALSE) {
            ESP_LOGW(TAG, "couldn't give sms_mutex");
        }

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

int drv_sms_get_value(enum SmsNum sms_num, TickType_t wait_ticks) {

    if (sms_num < 0 || sms_num >= SMS_END) {
        ESP_LOGW(TAG, "sms number out of range.");
        return -1;
    }

    int status = xSemaphoreTake(sms_mutex, wait_ticks);
    if (status == pdFALSE) {
        return -1;
    }
    int sms_val = sms_values[sms_num];
    status = xSemaphoreGive(sms_mutex);

    if (status == pdFALSE) {
        return -1;
    }

    return sms_val;

}

esp_err_t drv_sms_set_value(enum SmsNum sms_num, int value, TickType_t wait_ticks) {

    if (sms_num < 0 || sms_num >= SMS_END) {
        ESP_LOGW(TAG, "sms number out of range.");
        return ESP_FAIL;
    }

    int status = xSemaphoreTake(sms_mutex, wait_ticks);
    if (status == pdFALSE) {
        return ESP_FAIL;
    }
    sms_values[sms_num] = value;
    status = xSemaphoreGive(sms_mutex);

    if (status == pdFALSE) {
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t drv_sms_init(void) {

    sms_mutex = xSemaphoreCreateMutex();
    int i, status;

    status = xSemaphoreTake(sms_mutex, portMAX_DELAY);
    if (status == pdFALSE) {
        ESP_LOGW(TAG, "couldn't take sms_mutex");
    }
    for (i = 0; i < SMS_END; i++) {
        sms_values[i] = 0;
    }
    status = xSemaphoreGive(sms_mutex);

    if (status == pdFALSE) {
        ESP_LOGW(TAG, "couldn't give sms_mutex");
    }

    xTaskCreate(sms_task, "sms task", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "initialized sms sensors");
    return ESP_OK;
}

esp_err_t drv_sms_deinit(void) {
    return ESP_OK;
}
