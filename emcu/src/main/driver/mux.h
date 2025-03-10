/*
 * we will only be using adc1 for now.
 * TODO: lookup ADC channel from MUX_READ_PIN
 * */
#define MUX_ADC_CHANNEL      ADC1_CHANNEL_4

/*
 * always call these before calling any other functions, except drv_mux_init()
 * */
esp_err_t drv_mux_take_access(TickType_t wait_ticks);
esp_err_t drv_mux_give_access(void);

int drv_mux_read_analog(void);

int drv_mux_read_digital(void);

esp_err_t drv_mux_push_addr(uint8_t addr);

esp_err_t drv_mux_init(void);
