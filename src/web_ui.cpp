#include "web_ui.h"
#include "crypto_lab.h"
#include <WiFi.h>

static String jsonEscape(const String& input) {
    static const char hex[] = "0123456789abcdef";
    String output;
    output.reserve(input.length() + 8);
    for (size_t i = 0; i < input.length(); ++i) {
        const uint8_t c = static_cast<uint8_t>(input[i]);
        switch (c) {
            case '"': output += "\\\""; break;
            case '\\': output += "\\\\"; break;
            case '\b': output += "\\b"; break;
            case '\f': output += "\\f"; break;
            case '\n': output += "\\n"; break;
            case '\r': output += "\\r"; break;
            case '\t': output += "\\t"; break;
            default:
                if (c < 0x20) {
                    output += "\\u00";
                    output += hex[(c >> 4) & 0x0F];
                    output += hex[c & 0x0F];
                } else {
                    output += static_cast<char>(c);
                }
                break;
        }
    }
    return output;
}

void WebUI::begin(EventLogger* logger,
                  std::vector<NetworkInfo>* networks,
                  String* lastHash,
                  NetworkMonitor* monitor,
                  AppState* state,
                  const String* password) {
    eventLogger = logger;
    scanResults = networks;
    hashValue = lastHash;
    networkMonitor = monitor;
    appState = state;
    accessPassword = password;
    routes();
    server.begin();
}

bool WebUI::authorize() {
    if (accessPassword && server.authenticate("admin", accessPassword->c_str())) {
        return true;
    }
    server.requestAuthentication(BASIC_AUTH, "ESP32 CyberLab", "Valid dashboard credentials required");
    return false;
}

void WebUI::routes() {
    server.on("/", HTTP_GET, [this]() {
        if (!authorize()) return;
        String page = R"HTML(
<!doctype html><html><head><meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32 CyberLab</title>
<style>
body{font-family:system-ui;background:#101827;color:#e5edf7;margin:24px}
.card{background:#1b293b;padding:18px;border-radius:12px;margin:12px 0}
button{background:#16a085;color:white;border:0;padding:10px 14px;border-radius:8px;cursor:pointer}
table{width:100%;border-collapse:collapse}td,th{padding:8px;border-bottom:1px solid #35465c;text-align:left}
small{color:#9fb0c5}
input{padding:9px;border-radius:6px}
</style></head><body><h1>ESP32 CyberLab</h1><small>Local lab dashboard · v0.1.0</small>
<div class="card"><h2>Wi-Fi scan results</h2><button onclick="location.href='/scan'">Run scan</button><div id="networks">Loading…</div></div>
<div class="card"><h2>SHA-256</h2><form action="/hash" method="get"><input name="text" maxlength="256" placeholder="Text to hash" required><button>Hash</button></form><p id="hash"></p></div>
<div class="card"><h2>Recent events</h2><pre id="logs">Loading…</pre><a href="/logs">Download log</a></div>
<script>
function cell(row,value){const td=document.createElement('td');td.textContent=String(value);row.appendChild(td);}
fetch('/api/networks').then(r=>{if(!r.ok)throw Error('Request failed');return r.json();}).then(items=>{
 const table=document.createElement('table'),head=document.createElement('tr');
 ['SSID','RSSI','Channel','Security'].forEach(v=>cell(head,v));table.appendChild(head);
 items.forEach(n=>{const row=document.createElement('tr');cell(row,n.ssid);cell(row,n.rssi+' dBm');cell(row,n.channel);cell(row,n.auth);table.appendChild(row);});
 const target=document.getElementById('networks');target.replaceChildren(table);
}).catch(()=>{document.getElementById('networks').textContent='Unable to load scan results';});
fetch('/api/logs').then(r=>{if(!r.ok)throw Error('Request failed');return r.text();}).then(t=>document.getElementById('logs').textContent=t.slice(-1800)).catch(()=>{document.getElementById('logs').textContent='Unable to load logs';});
fetch('/api/status').then(r=>{if(!r.ok)throw Error('Request failed');return r.json();}).then(s=>{document.getElementById('hash').textContent=s.last_hash?'SHA-256: '+s.last_hash:'No hash generated yet';}).catch(()=>{});
</script></body></html>)HTML";
        server.send(200, "text/html; charset=utf-8", page);
    });

    server.on("/scan", HTTP_GET, [this]() {
        if (!authorize()) return;
        if (!networkMonitor || !scanResults || !appState) {
            server.send(503, "text/plain", "Scanner unavailable");
            return;
        }
        if (appState->wifiScanRunning) {
            server.sendHeader("Location", "/");
            server.send(303);
            return;
        }

        appState->wifiScanRunning = true;
        const int found = networkMonitor->scan(*scanResults);
        appState->networkCount = found > 0 ? found : 0;
        appState->lastScanMs = millis();
        appState->wifiScanRunning = false;
        if (eventLogger) {
            eventLogger->log(found >= 0 ? "INFO" : "ERROR",
                              "Dashboard Wi-Fi scan finished; networks=" + String(found));
        }
        server.sendHeader("Location", "/");
        server.send(303);
    });

    server.on("/api/networks", HTTP_GET, [this]() {
        if (!authorize()) return;
        String json = "[";
        if (scanResults) {
            for (size_t i = 0; i < scanResults->size(); ++i) {
                const auto& n = (*scanResults)[i];
                if (i) json += ",";
                json += "{\"ssid\":\"" + jsonEscape(n.ssid) + "\",\"rssi\":" + String(n.rssi);
                json += ",\"channel\":" + String(n.channel);
                json += ",\"auth\":\"" + jsonEscape(NetworkMonitor::authName(n.auth)) + "\"}";
            }
        }
        json += "]";
        server.send(200, "application/json; charset=utf-8", json);
    });

    server.on("/hash", HTTP_GET, [this]() {
        if (!authorize()) return;
        String input = server.arg("text");
        if (input.length() > 256) input = input.substring(0, 256);
        if (hashValue) *hashValue = sha256Hex(input);
        if (eventLogger) eventLogger->log("INFO", "SHA-256 computed via dashboard");
        server.sendHeader("Location", "/");
        server.send(303);
    });

    server.on("/api/logs", HTTP_GET, [this]() {
        if (!authorize()) return;
        server.send(200, "text/plain; charset=utf-8", eventLogger ? eventLogger->readAll() : "Logger unavailable");
    });

    server.on("/logs", HTTP_GET, [this]() {
        if (!authorize()) return;
        server.sendHeader("Content-Disposition", "attachment; filename=cyberlab-events.csv");
        server.send(200, "text/csv; charset=utf-8", eventLogger ? eventLogger->readAll() : "");
    });

    server.on("/api/status", HTTP_GET, [this]() {
        if (!authorize()) return;
        String json = "{\"uptime_ms\":" + String(millis()) +
                      ",\"free_heap\":" + String(ESP.getFreeHeap()) +
                      ",\"last_hash\":\"" + jsonEscape(hashValue ? *hashValue : "") + "\"}";
        server.send(200, "application/json; charset=utf-8", json);
    });

    server.onNotFound([this]() {
        if (!authorize()) return;
        server.send(404, "text/plain", "Not found");
    });
}

void WebUI::handleClient() {
    server.handleClient();
}
