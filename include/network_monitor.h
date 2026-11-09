#pragma once
#include <Arduino.h>
#include <vector>
#include "app_state.h"

class NetworkMonitor {
public:
    static constexpr int SCAN_RUNNING = -1;
    static constexpr int SCAN_FAILED = -2;

    void begin();
    bool startScan();
    // Returns SCAN_RUNNING, SCAN_FAILED, or the number of networks found.
    int pollScan(std::vector<NetworkInfo>& results);
    static String authName(wifi_auth_mode_t mode);
};
