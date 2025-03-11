enum UsoNum {
    USO_0 = 0,
    USO_1,
    USO_END
};

int drv_uso_get_value(enum UsoNum uso_num, TickType_t wait_ticks);

esp_err_t drv_uso_set_value(enum UsoNum uso_num, int value, TickType_t wait_ticks);

esp_err_t drv_uso_init(void);

esp_err_t drv_uso_deinit(void);
