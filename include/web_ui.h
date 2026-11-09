#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include <vector>
#include "app_state.h"
#include "logger.h"

class WebUI {
public:
    void begin(EventLogger* logger, std::vector<NetworkInfo>* networks, String* lastHash);
    void handleClient();
private:
    WebServer server{80};
    EventLogger* eventLogger = nullptr;
    std::vector<NetworkInfo>* scanResults = nullptr;
    String* hashValue = nullptr;
    void routes();
};
