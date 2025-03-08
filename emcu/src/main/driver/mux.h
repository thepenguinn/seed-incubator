#define DRV_MUX_CLK_GPIO   GPIO_NUM_4
#define DRV_MUX_DATA_GPIO GPIO_NUM_18
#define DRV_MUX_EN_GPIO   GPIO_NUM_19

esp_err_t drv_mux_push_addr(uint8_t addr);

esp_err_t drv_mux_init(void);
