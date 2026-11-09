#pragma once
#include <Arduino.h>
#include <WiFi.h>

struct NetworkInfo {
    String ssid;
    int32_t rssi = 0;
    wifi_auth_mode_t auth = WIFI_AUTH_OPEN;
    int32_t channel = 0;
};

struct AppState {
    bool oledReady = false;
    bool wifiScanRunning = false;
    unsigned long lastScanMs = 0;
    int networkCount = 0;
    String lastHash;
};
