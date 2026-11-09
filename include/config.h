#pragma once

// Adjust these pins to match your ESP32-S3 board and wiring.
#define OLED_SDA 8
#define OLED_SCL 9
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

#define WIFI_SCAN_INTERVAL_MS 30000UL
#define EVENT_LOG_PATH "/events.log"
#define MAX_LOG_BYTES 12000
