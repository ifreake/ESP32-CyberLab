#include "network_monitor.h"
#include <WiFi.h>

void NetworkMonitor::begin() {
    // Preserve the AP mode selected by setup(); disconnect only the station.
    WiFi.disconnect(false, true);
    delay(100);
}

bool NetworkMonitor::startScan() {
    if (WiFi.scanComplete() == WIFI_SCAN_RUNNING) return false;
    WiFi.scanDelete();
    const int result = WiFi.scanNetworks(true, true);
    return result == WIFI_SCAN_RUNNING || result >= 0;
}

int NetworkMonitor::pollScan(std::vector<NetworkInfo>& results) {
    const int count = WiFi.scanComplete();
    if (count == WIFI_SCAN_RUNNING) return SCAN_RUNNING;
    if (count < 0) {
        WiFi.scanDelete();
        return SCAN_FAILED;
    }

    std::vector<NetworkInfo> freshResults;
    freshResults.reserve(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) {
        NetworkInfo item;
        item.ssid = WiFi.SSID(i);
        item.rssi = WiFi.RSSI(i);
        item.auth = WiFi.encryptionType(i);
        item.channel = WiFi.channel(i);
        freshResults.push_back(item);
    }
    WiFi.scanDelete();
    results.swap(freshResults);
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
