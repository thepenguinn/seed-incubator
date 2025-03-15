#include<stdio.h>
#include<stdarg.h>

#include "esp_err.h"
#include "esp_log.h"

void esp_logger(const char *tag, int log_level, const char *fmt, ...) {

	va_list ap;

	if (log_level == ESP_LOG_ERROR)
		fprintf(stderr, "ERROR: ");
	else if (log_level == ESP_LOG_WARN)
		fprintf(stderr, "WARN:  ");
	else if (log_level == ESP_LOG_INFO)
		fprintf(stderr, "INFO:  ");
	else if (log_level == ESP_LOG_DEBUG)
		fprintf(stderr, "DEBUG: ");

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	fprintf(stderr, "\n");

}
