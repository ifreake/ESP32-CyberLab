#include "network_monitor.h"
#include <WiFi.h>

void NetworkMonitor::begin() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true, true);
    delay(100);
}

int NetworkMonitor::scan(std::vector<NetworkInfo>& results) {
    results.clear();
    int count = WiFi.scanNetworks(false, true);
    if (count < 0) return count;

    for (int i = 0; i < count; ++i) {
        NetworkInfo item;
        item.ssid = WiFi.SSID(i);
        item.rssi = WiFi.RSSI(i);
        item.auth = WiFi.encryptionType(i);
        item.channel = WiFi.channel(i);
        results.push_back(item);
    }
    WiFi.scanDelete();
    return static_cast<int>(results.size());
}

String NetworkMonitor::authName(wifi_auth_mode_t mode) {
    switch (mode) {
        case WIFI_AUTH_OPEN: return "OPEN";
        case WIFI_AUTH_WEP: return "WEP";
        case WIFI_AUTH_WPA_PSK: return "WPA";
        case WIFI_AUTH_WPA2_PSK: return "WPA2";
        case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2";
        case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2-ENT";
        case WIFI_AUTH_WPA3_PSK: return "WPA3";
        case WIFI_AUTH_WPA2_WPA3_PSK: return "WPA2/WPA3";
        default: return "UNKNOWN";
    }
}
