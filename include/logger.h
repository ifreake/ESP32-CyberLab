#pragma once
#include <Arduino.h>

class EventLogger {
public:
    bool begin();
    void log(const String& level, const String& message);
    String readAll();
    void clear();
private:
    bool ready = false;
};
