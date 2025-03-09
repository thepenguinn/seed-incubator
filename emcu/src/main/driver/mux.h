esp_err_t drv_mux_take_access(TickType_t wait_ticks);
esp_err_t drv_mux_give_access(void);

esp_err_t drv_mux_push_addr(uint8_t addr);

esp_err_t drv_mux_init(void);
