enum LdrNum {
    LDR_0 = 0,
    LDR_1,
    LDR_2,
    LDR_END
};

int drv_ldr_get_value(enum LdrNum ldr_num, TickType_t wait_ticks);

esp_err_t drv_ldr_set_value(enum LdrNum ldr_num, int value, TickType_t wait_ticks);

esp_err_t drv_ldr_init(void);

esp_err_t drv_ldr_deinit(void);
