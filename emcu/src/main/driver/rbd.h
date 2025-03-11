/*
 * always call these before calling any other functions, except drv_rbd_init()
 * */
esp_err_t drv_rbd_take_access(TickType_t wait_ticks);
esp_err_t drv_rbd_give_access(void);

esp_err_t drv_rbd_push_data(uint16_t data);

uint16_t drv_rbd_get_data(void);
esp_err_t drv_rbd_init(void);
