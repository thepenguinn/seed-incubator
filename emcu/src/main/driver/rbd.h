/*
 * always call these before calling any other functions, except drv_rbd_init()
 * */
esp_err_t drv_rbd_take_access(TickType_t wait_ticks);
esp_err_t drv_rbd_give_access(void);

esp_err_t drv_rbd_push_data(uint16_t data);

esp_err_t drv_rbd_peltier(int state);
esp_err_t drv_rbd_fan_0(int state);
esp_err_t drv_rbd_fan_1(int state);
esp_err_t drv_rbd_panel_0(int state);

uint16_t drv_rbd_get_data(void);
esp_err_t drv_rbd_init(void);
