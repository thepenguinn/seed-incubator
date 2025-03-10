enum SmsNum {
    SMS_0 = 0,
    SMS_1,
    SMS_2,
    SMS_3,
    SMS_END
};

int drv_sms_get_value(enum SmsNum sms_num, TickType_t wait_ticks);

esp_err_t drv_sms_set_value(enum SmsNum sms_num, int value, TickType_t wait_ticks);

esp_err_t drv_sms_init(void);

esp_err_t drv_sms_deinit(void);
