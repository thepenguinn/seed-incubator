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
    SUB_CMD_END,
};

/*
 * @brief initializes tcp server
 *
 * @param void
 * @return
 *      - ESP_OK: succeeded
 *      - ESP_FAIL: failed for some reason
 * */
esp_err_t tcp_server_init(void);
