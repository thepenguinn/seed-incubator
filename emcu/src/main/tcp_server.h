/* for tcp server */
#define PORT                 10898
#define KEEPALIVE_IDLE           5
#define KEEPALIVE_INTERVAL       5
#define KEEPALIVE_COUNT          3

/*#define TCP_KILLED_BIT     BIT0*/

enum SubCmd {
    SUB_CMD_UNKNOWN = 0,
    SUB_CMD_HMODE,
    SUB_CMD_CMODE,
    SUB_CMD_MONITOR,
    SUB_CMD_FANS,
    SUB_CMD_PELTIER,
    SUB_CMD_MUX,
    SUB_CMD_RBD,
    SUB_CMD_MONITOR_TEMP,
    SUB_CMD_MONITOR_HUME,
    SUB_CMD_MONITOR_LDR,
    SUB_CMD_MONITOR_SMS,
    SUB_CMD_MONITOR_USO,
    SUB_CMD_TUI,
    SUB_CMD_PANEL_0,
    SUB_CMD_HUME,
    SUB_CMD_FAN_0,
    SUB_CMD_FAN_1,
    SUB_CMD_LIGHT_0,
    SUB_CMD_LIGHT_1,
    SUB_CMD_END,
};

/*
 * TODO: make this tcp_server more event driven
 * */

/*
 * @brief initializes tcp server
 *
 * @param void
 * @return
 *      - ESP_OK: succeeded
 *      - ESP_FAIL: failed for some reason
 *
 *  @note wifi and netif should be initialized before calling this function
 * */
esp_err_t tcp_server_init(void);

/*
 * @brief de-initializes tcp server
 *
 * @param void
 * @return
 *      - ESP_OK: succeeded
 *      - ESP_FAIL: failed for some reason
 * */
esp_err_t tcp_server_deinit(void);
