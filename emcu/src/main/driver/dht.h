enum DhtNum {
    DHT_0 = 0,
    DHT_1,
    DHT_END
};

enum DhtData {
    DHT_DATA_TEMP = 0,
    DHT_DATA_HUME,
    DHT_DATA_END,
};

int drv_dht_get_value(
    enum DhtNum dht_num, enum DhtData dht_data, TickType_t wait_ticks);

esp_err_t drv_dht_set_value(
    enum DhtNum dht_num, enum DhtData dht_data, int value, TickType_t wait_ticks);

esp_err_t drv_dht_init(void);

esp_err_t drv_dht_deinit(void);
