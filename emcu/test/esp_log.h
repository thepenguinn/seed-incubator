enum EspLogLevel {
	ESP_LOG_ERROR,
	ESP_LOG_WARN,
	ESP_LOG_INFO,
	ESP_LOG_DEBUG,
};

#define ESP_LOGE( tag, format, ... ) esp_logger(tag, ESP_LOG_ERROR, format, ##__VA_ARGS__)
#define ESP_LOGW( tag, format, ... ) esp_logger(tag, ESP_LOG_WARN,  format, ##__VA_ARGS__)
#define ESP_LOGI( tag, format, ... ) esp_logger(tag, ESP_LOG_INFO,  format, ##__VA_ARGS__)
#define ESP_LOGD( tag, format, ... ) esp_logger(tag, ESP_LOG_DEBUG, format, ##__VA_ARGS__)

void esp_logger(const char *tag, int log_level, const char *fmt, ...);
