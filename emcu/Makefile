# hehe, a build system for another build system

ESP_PORT := $(shell echo "socket://${ESP_IP}:8080")

build: src/main/emcu.c
	cd src && idf.py build

build_test:

flash: build
	cd src/build && python -m esptool --chip esp32 -b 115200 --before no_reset --after no_reset --port $(ESP_PORT) write_flash "@flash_args"

monitor:
	cd src && idf.py -p $(ESP_PORT) monitor

.PHONY: build build_test flash monitor
