#include "logger.h"
#include "config.h"
#include <LittleFS.h>

bool EventLogger::begin() {
    ready = LittleFS.begin(true);
    if (ready && !LittleFS.exists(EVENT_LOG_PATH)) {
        File f = LittleFS.open(EVENT_LOG_PATH, FILE_WRITE);
        if (f) {
            f.println("timestamp_ms,level,message");
            f.close();
        }
    }
    return ready;
}

void EventLogger::log(const String& level, const String& message) {
    if (!ready) return;
    File f = LittleFS.open(EVENT_LOG_PATH, FILE_APPEND);
    if (!f) return;

    String safe = message;
    safe.replace(",", ";");
    safe.replace("\n", " ");
    f.printf("%lu,%s,%s\n", millis(), level.c_str(), safe.c_str());
    f.close();

    File r = LittleFS.open(EVENT_LOG_PATH, FILE_READ);
    if (r && r.size() > MAX_LOG_BYTES) {
        r.close();
        String contents = readAll();
        int cut = contents.indexOf('\n', contents.length() / 2);
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
