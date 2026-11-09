#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include <vector>
#include "app_state.h"
#include "logger.h"
#include "network_monitor.h"

class WebUI {
public:
    void begin(EventLogger* logger,
               std::vector<NetworkInfo>* networks,
               String* lastHash,
               NetworkMonitor* networkMonitor,
               AppState* state,
               const String& accessPassword);
    void handleClient();
private:
    WebServer server{80};
    EventLogger* eventLogger = nullptr;
    std::vector<NetworkInfo>* scanResults = nullptr;
    String* hashValue = nullptr;
    NetworkMonitor* networkMonitor = nullptr;
    AppState* appState = nullptr;
    String accessPassword;\n    String csrfToken;
    bool authorize();
    bool authorizeMutation();
    void routes();
};
