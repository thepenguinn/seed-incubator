/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h" // maynot need
#include "esp_event.h" // maynot need
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include <lwip/netdb.h>

#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "emcu.h"
#include "DHT.h"

static SemaphoreHandle_t temp_hume_mutex;
static SemaphoreHandle_t light_mutex;

/*static SemaphoreHandle_t reserv_mutex;*/
/*static SemaphoreHandle_t soil_moist_mutex;*/

static char temp_value[6];
static char hume_value[6];
static char light_value[6];

/*static float light_value = 0;*/
/*static float soil_moist_value = 0;*/
/*static float reserv_value = 0;*/

static SemaphoreHandle_t wifi_conn_mutex;
static int wifi_connected = pdFALSE;

static char ip_addr[128];

static const char *TCP_TAG = "tcp server";
static const char *DHT_TAG = "dht task";
static const char *LIT_TAG = "lit task";

static int s_retry_num = 0;

static int serve_client(int client_sock);

void temp_hume_task(void *pvParameters);

void tcp_server_task(void *pvParameters);

void wifi_init_sta(void);

static void check_efuse(void) {
#if CONFIG_IDF_TARGET_ESP32
    //Check if TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("eFuse Two Point: NOT supported\n");
    }
    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printf("eFuse Vref: Supported\n");
    } else {
        printf("eFuse Vref: NOT supported\n");
    }
#else
#error "This example is configured for ESP32/ESP32S2."
#endif
}

static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
}

static int send_all(int client_sock, char *data, size_t len) {

    int ret;

    while (len > 0) {
        ret = send(client_sock, data, 1, 0);
        if (ret < 0) {
            ESP_LOGE(TCP_TAG, "Error occurred during receiving: errno %d", errno);
            return 1;
        } else if (ret == 0) {
            ESP_LOGW(TCP_TAG, "Connection closed");
            return 1;
        }
        data++;
        len--;
    }
    return 0;
}

static int serve_client(int client_sock) {

    /*
     * every command will be two bytes, response will be differently sized.
     * monitor
     * */

    char buf[2];
    int len;

    while (1) {
        len = recv(client_sock, buf, sizeof(buf) - 1, 0);
        if (len < 0) {
            ESP_LOGE(TCP_TAG, "Error occurred during receiving: errno %d", errno);
            break;
        } else if (len == 0) {
            ESP_LOGW(TCP_TAG, "Connection closed");
            break;
        }
        recv(client_sock, buf + 1, sizeof(buf) - 1, 0);

        if (len < 0) {
            ESP_LOGE(TCP_TAG, "Error occurred during receiving: errno %d", errno);
            break;
        } else if (len == 0) {
            ESP_LOGW(TCP_TAG, "Connection closed");
            break;
        } else {
            if ((int) buf[0] == SUB_CMD_MONITOR) {
                xSemaphoreTake(temp_hume_mutex, portMAX_DELAY);
                if (send_all(client_sock, temp_value, sizeof(temp_value))) {
                    xSemaphoreGive(temp_hume_mutex);
                    break;
                }
                if (send_all(client_sock, hume_value, sizeof(hume_value))) {
                    xSemaphoreGive(temp_hume_mutex);
                    break;
                }
                xSemaphoreGive(temp_hume_mutex);

                xSemaphoreTake(light_mutex, portMAX_DELAY);
                if (send_all(client_sock, light_value, sizeof(light_value))) {
                    xSemaphoreGive(light_mutex);
                    break;
                }
                xSemaphoreGive(light_mutex);
            }
        }
    }

    return 0;

}

void tcp_server_task(void *pvParameters) {

    EventBits_t bits;
    int keepAlive = 1;
    int keepIdle = KEEPALIVE_IDLE;
    int keepInterval = KEEPALIVE_INTERVAL;
    int keepCount = KEEPALIVE_COUNT;
    int err;
    struct sockaddr_in server_addr;

    do {

        ESP_LOGI(TCP_TAG, "Waiting for wifi to connect");
        bits = xEventGroupWaitBits(
            wifi_event_group,
            WIFI_CONNECTED_BIT,
            pdTRUE,
            pdFALSE,
            portMAX_DELAY);

        if (!(bits & WIFI_CONNECTED_BIT)) {
            ESP_LOGE(TCP_TAG, "UNEXPECTED EVENT");
            continue;
        }

        ESP_LOGI(TCP_TAG, "Starting tcp server");

        bzero(&server_addr, sizeof(server_addr));

        server_addr.sin_addr.s_addr = inet_addr(ip_addr); // address from event handler
        /*server_addr.sin_addr.s_addr = htonl(INADDR_ANY);*/
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);

        int listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (listen_sock < 0) {
            ESP_LOGE(TCP_TAG, "Unable to create socket: errno %d", errno);
            continue;
        }
        int opt = 1;
        setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        int flags = fcntl(listen_sock, F_GETFL, 0);
        flags = flags | O_NONBLOCK;
        err = fcntl(listen_sock, F_SETFL, flags);
        if (err == -1) {
            ESP_LOGE(TCP_TAG, "Failed to set listening socket to NON BLOCKING, errno: %d", errno);
            goto CLEAN_UP;
        }

        ESP_LOGI(TCP_TAG, "Socket created");

        err = bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (err != 0) {
            ESP_LOGE(TCP_TAG, "Socket unable to bind: errno %d", errno);
            goto CLEAN_UP;
        }
        ESP_LOGI(TCP_TAG, "Socket bound, port %d", PORT);

        err = listen(listen_sock, 1);
        if (err != 0) {
            ESP_LOGE(TCP_TAG, "Error occurred during listen: errno %d", errno);
            goto CLEAN_UP;
        }

        ESP_LOGI(TCP_TAG, "Socket listening");

        /*
         * waiting for android
         * */

        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client_sock;

        for ( ;; ) {

            // keep polling
            for ( ;; ) {
                xSemaphoreTake(wifi_conn_mutex, portMAX_DELAY);
                if (wifi_connected == pdFALSE) {
                    // connection to wifi lost, goto CLEAN_UP and wait for wifi to connect
                    ESP_LOGW(TCP_TAG, "Seems like wifi has disconnected");
                    xSemaphoreGive(wifi_conn_mutex);
                    goto CLEAN_UP;
                }
                xSemaphoreGive(wifi_conn_mutex);
                // still connected, looking for connections.
                client_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &addr_len);
                if (client_sock < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                    ESP_LOGI(TCP_TAG, "waiting for 500ms before next poll");
                    vTaskDelay(500 / portTICK_PERIOD_MS);
                    continue;
                } else {
                    break;
                }
            }
            if (client_sock < 0) {
                // accept failed because of some other reason.
                ESP_LOGE(TCP_TAG, "Unable to accept connection: errno %d", errno);
                continue;
            }

            // Set tcp keepalive option
            setsockopt(client_sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
            setsockopt(client_sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
            setsockopt(client_sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
            setsockopt(client_sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));

            ESP_LOGI(TCP_TAG, "GOT ANDROID CONNECTION.");

            serve_client(client_sock);

            xSemaphoreTake(wifi_conn_mutex, portMAX_DELAY);
            if (wifi_connected == pdFALSE) {
                // connection to wifi lost, breaking from this loop
                ESP_LOGW(TCP_TAG, "Seems like wifi has disconnected");
                xSemaphoreGive(wifi_conn_mutex);
                break;
            }
            ESP_LOGW(TCP_TAG, "Connection to android ended, waiting for next");
            xSemaphoreGive(wifi_conn_mutex);

        }

    CLEAN_UP:
        close(listen_sock);

    } while (1);

}

void temp_hume_task(void *pvParameters) {

    setDHTgpio(GPIO_NUM_4);
    ESP_LOGI(DHT_TAG, "Starting DHT Task\n\n");

    for ( ;; ) {

        ESP_LOGI(DHT_TAG, "=== Reading DHT ===\n");
        int ret = readDHT();

        errorHandler(ret);

        ESP_LOGI(DHT_TAG, "Hum: %.1f Tmp: %.1f\n", getHumidity(), getTemperature());

        xSemaphoreTake(temp_hume_mutex, portMAX_DELAY);
        snprintf(temp_value, sizeof(temp_value) - 1, "%.1f", getTemperature());
        snprintf(hume_value, sizeof(hume_value) - 1, "%.1f", getHumidity());
        xSemaphoreGive(temp_hume_mutex);

        vTaskDelay(3000 / portTICK_PERIOD_MS);

    }

}

void light_task(void *pvParameters) {

    //Continuously sample ADC1
    while (1) {
        uint32_t adc_reading = 0;
        //Multisampling
        for (int i = 0; i < NO_OF_SAMPLES; i++) {
            if (unit == ADC_UNIT_1) {
                adc_reading += adc1_get_raw((adc1_channel_t)channel);
            } else {
                int raw;
                adc2_get_raw((adc2_channel_t)channel, width, &raw);
                adc_reading += raw;
            }
        }
        adc_reading /= NO_OF_SAMPLES;
        //Convert adc_reading to voltage in mV
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
        printf("Raw: %ld\tVoltage: %ldmV\n", adc_reading, voltage);
        xSemaphoreTake(light_mutex, portMAX_DELAY);
        snprintf(light_value, sizeof(light_value) - 1, "%ld", voltage);
        xSemaphoreGive(light_mutex);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void) {
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(WIFI_TAG, "ESP_WIFI_MODE_STA");

    wifi_init_sta();

    // for ADC used with ldr

    //Check if Two Point or Vref are burned into eFuse
    check_efuse();

    //Configure ADC
    if (unit == ADC_UNIT_1) {
        adc1_config_width(width);
        adc1_config_channel_atten(channel, atten);
    } else {
        adc2_config_channel_atten((adc2_channel_t)channel, atten);
    }

    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);

    ///

    esp_rom_gpio_pad_select_gpio(GPIO_NUM_4);

    temp_hume_mutex = xSemaphoreCreateMutex();
    light_mutex = xSemaphoreCreateMutex();

    xTaskCreate(temp_hume_task, "temp_hume_task", 4096, NULL, 5, NULL);

    xTaskCreate(light_task, "light_task", 4096, NULL, 5, NULL);

}
