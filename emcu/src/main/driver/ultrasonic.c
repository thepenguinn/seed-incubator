#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"

#include "driver.h"
#include "pins.h"
#include "mux.h"

#include "lib/ultrasonic.h"

#include "ultrasonic.h"

/* 5m is max */
#define MAX_DISTANCE_CM 100

static const char *TAG = "uso driver";

static SemaphoreHandle_t uso_mutex;

static int uso_values[USO_END];

static const int uso_echo_pins[USO_END] = {
    [USO_0] = GPIO_NUM_34,
    [USO_1] = GPIO_NUM_35,
};

static void uso_task(void *pvParameters);

static void uso_task(void *pvParameters) {

    ultrasonic_sensor_t sensor;

    int uso_v[USO_END];
    int i, status;
    float distance;
    esp_err_t res;

    while (1) {
        drv_mux_take_access(portMAX_DELAY);

        for (i = 0; i < USO_END; i++) {
            drv_mux_push_addr(MUX_ADDR_USO_OFFSET + i);

            /*
             * this will deinit analog read and enables digital read if
             * not in digital read mode
             * */
            drv_mux_read_digital();

            sensor.trigger_pin = MUX_READ_PIN;
            sensor.echo_pin = uso_echo_pins[i];

            ultrasonic_init(&sensor);

            vTaskDelay(200 / portTICK_PERIOD_MS);

            res = ultrasonic_measure(&sensor, MAX_DISTANCE_CM, &distance);
            if (res != ESP_OK) {
                ESP_LOGE(TAG, "Error %d: ", res);
                switch (res) {
                    case ESP_ERR_ULTRASONIC_PING:
                        ESP_LOGE(TAG, "Cannot ping (device is in invalid state)");
                        break;
                    case ESP_ERR_ULTRASONIC_PING_TIMEOUT:
                        ESP_LOGE(TAG, "Ping timeout (no device found)");
                        break;
                    case ESP_ERR_ULTRASONIC_ECHO_TIMEOUT:
                        ESP_LOGE(TAG, "Echo timeout (i.e. distance too big)");
                        break;
                    default:
                        ESP_LOGE(TAG, "%s", esp_err_to_name(res));
                }
            } else {
                uso_v[i] = (int) distance * 100;
                ESP_LOGW(TAG, "Got a read %0.02f cm", distance * 100);
            }
        }

        drv_mux_give_access();

        status = xSemaphoreTake(uso_mutex, portMAX_DELAY);
        if (status == pdFALSE) {
            ESP_LOGW(TAG, "couldn't take uso_mutex");
        }
        for (i = 0; i < USO_END; i++) {
            uso_values[i] = uso_v[i];
        }
        status = xSemaphoreGive(uso_mutex);

        if (status == pdFALSE) {
            ESP_LOGW(TAG, "couldn't give uso_mutex");
        }

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

int drv_uso_get_value(enum UsoNum uso_num, TickType_t wait_ticks) {

    if (uso_num < 0 || uso_num >= USO_END) {
        ESP_LOGW(TAG, "uso number out of range.");
        return -1;
    }

    int status = xSemaphoreTake(uso_mutex, wait_ticks);
    if (status == pdFALSE) {
        return -1;
    }
    int uso_val = uso_values[uso_num];
    status = xSemaphoreGive(uso_mutex);

    if (status == pdFALSE) {
        return -1;
    }

    return uso_val;

}

esp_err_t drv_uso_set_value(enum UsoNum uso_num, int value, TickType_t wait_ticks) {

    if (uso_num < 0 || uso_num >= USO_END) {
        ESP_LOGW(TAG, "uso number out of range.");
        return ESP_FAIL;
    }

    int status = xSemaphoreTake(uso_mutex, wait_ticks);
    if (status == pdFALSE) {
        return ESP_FAIL;
    }
    uso_values[uso_num] = value;
    status = xSemaphoreGive(uso_mutex);

    if (status == pdFALSE) {
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t drv_uso_init(void) {

    uso_mutex = xSemaphoreCreateMutex();
    int i, status;

    status = xSemaphoreTake(uso_mutex, portMAX_DELAY);
    if (status == pdFALSE) {
        ESP_LOGW(TAG, "couldn't take uso_mutex");
    }
    for (i = 0; i < USO_END; i++) {
        uso_values[i] = 0;
    }
    status = xSemaphoreGive(uso_mutex);

    if (status == pdFALSE) {
        ESP_LOGW(TAG, "couldn't give uso_mutex");
    }

    xTaskCreate(uso_task, "uso task", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "initialized uso sensors");
    return ESP_OK;
}

esp_err_t drv_uso_deinit(void) {
    return ESP_OK;
}
