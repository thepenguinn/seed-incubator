#define WIFI_SSID      "Daniel's Galaxy A50s"
#define WIFI_PASS      "thisisbaad"

#define WIFI_ESP_MAXIMUM_RETRY  10000

#if CONFIG_ESP_WPA3_SAE_PWE_HUNT_AND_PECK
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HUNT_AND_PECK
#define EXAMPLE_H2E_IDENTIFIER ""
#elif CONFIG_ESP_WPA3_SAE_PWE_HASH_TO_ELEMENT
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HASH_TO_ELEMENT
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#elif CONFIG_ESP_WPA3_SAE_PWE_BOTH
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#endif
#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_STA_IP_BIT BIT0

/*
 * @brief This should be called before calling any wifi_* functions.
 *
 * @param void
 *
 * @return
 *      - ESP_OK: succeeded
 *      - ESP_FAIL: failed for some reason.
 * */
esp_err_t wifi_init(void);

/*
 * @brief Initializes WIFI into station mode
 *
 * @param void
 * @return
 *      - ESP_OK: succeeded
 *      - ESP_FAIL: failed for some reason.
 * */
esp_err_t wifi_init_sta_mode(void);

/*
 * @brief Waits for esp to get an ip address, for the specified time.
 *
 * @param   wait_ticks freertos ticks to wait, portMAX_DELAY will cause
 *                     to wait indefinitely
 *
 * @return
 *      - ESP_OK: succeeded
 *      - ESP_FAIL: failed for some reason.
 * */
esp_err_t wifi_sta_ip_await(TickType_t wait_ticks);

/*
 * @brief checks whether wifi is connected to an access point or not.
 *
 * @param void
 *
 * @return
 *      - ESP_OK: connected to the access point.
 *      - ESP_FAIL: not connected to any access points.
 * */
esp_err_t wifi_is_sta_connected(void);
