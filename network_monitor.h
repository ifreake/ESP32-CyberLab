#pragma once
#include <Arduino.h>
#include <vector>
#include "app_state.h"

class NetworkMonitor {
public:
    void begin();
    int scan(std::vector<NetworkInfo>& results);
    static String authName(wifi_auth_mode_t mode);
};
