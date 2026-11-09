#include "logger.h"
#include "config.h"
#include <LittleFS.h>

static String csvEscape(const String& value) {
    String escaped;
    escaped.reserve(value.length() + 2);
    escaped += '"';
    for (size_t i = 0; i < value.length(); ++i) {
        const char c = value[i];
        if (c == '"') escaped += '"';
        escaped += c;
    }
    escaped += '"';
    return escaped;
}

bool EventLogger::begin() {
    // Never erase an existing log automatically when mounting fails.
    ready = LittleFS.begin(false);
    if (ready && !LittleFS.exists(EVENT_LOG_PATH)) {
        File f = LittleFS.open(EVENT_LOG_PATH, FILE_WRITE);
        if (f) {
            f.println("timestamp_ms,level,message");
            f.close();
        } else {
            ready = false;
        }
    }
    return ready;
}

void EventLogger::log(const String& level, const String& message) {
    if (!ready) return;
    File f = LittleFS.open(EVENT_LOG_PATH, FILE_APPEND);
    if (!f) return;

    f.printf("%lu,%s,%s\n", millis(), csvEscape(level).c_str(), csvEscape(message).c_str());
    f.close();

    File r = LittleFS.open(EVENT_LOG_PATH, FILE_READ);
    if (r && r.size() > MAX_LOG_BYTES) {
        r.close();
        String contents = readAll();
        const int cut = contents.indexOf('\n', contents.length() / 2);
        File w = LittleFS.open(EVENT_LOG_PATH, FILE_WRITE);
        if (w) {
            w.println("timestamp_ms,level,message");
            if (cut >= 0) w.print(contents.substring(cut + 1));
            w.close();
        }
    } else if (r) {
        r.close();
    }
}

String EventLogger::readAll() {
    if (!ready) return "Filesystem unavailable\n";
    File f = LittleFS.open(EVENT_LOG_PATH, FILE_READ);
    if (!f) return "No log file\n";
    String out = f.readString();
    f.close();
    return out;
}

void EventLogger::clear() {
    if (!ready) return;
    LittleFS.remove(EVENT_LOG_PATH);
    File f = LittleFS.open(EVENT_LOG_PATH, FILE_WRITE);
    if (f) {
        f.println("timestamp_ms,level,message");
        f.close();
    }
}
