#include "web_ui.h"
#include "network_monitor.h"
#include "crypto_lab.h"

static String htmlEscape(const String& in) {
    String out = in;
    out.replace("&", "&amp;");
    out.replace("<", "&lt;");
    out.replace(">", "&gt;");
    out.replace("\"", "&quot;");
    return out;
}

void WebUI::begin(EventLogger* logger, std::vector<NetworkInfo>* networks, String* lastHash) {
    eventLogger = logger;
    scanResults = networks;
    hashValue = lastHash;
    routes();
    server.begin();
}

void WebUI::routes() {
    server.on("/", HTTP_GET, [this]() {
        String page = R"HTML(
<!doctype html><html><head><meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32 CyberLab</title>
<style>
body{font-family:system-ui;background:#101827;color:#e5edf7;margin:24px}
.card{background:#1b293b;padding:18px;border-radius:12px;margin:12px 0}
button{background:#16a085;color:white;border:0;padding:10px 14px;border-radius:8px}
table{width:100%;border-collapse:collapse}td,th{padding:8px;border-bottom:1px solid #35465c;text-align:left}
small{color:#9fb0c5}
</style></head><body><h1>ESP32 CyberLab</h1><small>Local lab dashboard · v0.1.0</small>
<div class="card"><h2>Wi-Fi scan results</h2><button onclick="location.href='/scan'">Run scan</button><div id="networks">Loading…</div></div>
<div class="card"><h2>SHA-256</h2><form action="/hash" method="get"><input name="text" maxlength="256" placeholder="Text to hash" required><button>Hash</button></form><p id="hash"></p></div>
<div class="card"><h2>Recent events</h2><pre id="logs">Loading…</pre><a href="/logs">Download log</a></div>
<script>
fetch('/api/networks').then(r=>r.json()).then(a=>{let h='<table><tr><th>SSID</th><th>RSSI</th><th>Channel</th><th>Security</th></tr>';a.forEach(n=>h+=`<tr><td>${n.ssid}</td><td>${n.rssi} dBm</td><td>${n.channel}</td><td>${n.auth}</td></tr>`);document.getElementById('networks').innerHTML=h+'</table>'}).catch(()=>{});
fetch('/api/logs').then(r=>r.text()).then(t=>document.getElementById('logs').textContent=t.slice(-1800));
fetch('/api/status').then(r=>r.json()).then(s=>{document.getElementById('hash').textContent=s.last_hash ? 'SHA-256: '+s.last_hash : 'No hash generated yet';});
</script></body></html>)HTML";
        server.send(200, "text/html", page);
    });

    server.on("/scan", HTTP_GET, [this]() {
        server.sendHeader("Location", "/");
        server.send(303);
    });

    server.on("/api/networks", HTTP_GET, [this]() {
        String json = "[";
        if (scanResults) {
            for (size_t i = 0; i < scanResults->size(); ++i) {
                const auto& n = (*scanResults)[i];
                if (i) json += ",";
                json += "{\"ssid\":\"" + htmlEscape(n.ssid) + "\",\"rssi\":" + String(n.rssi);
                json += ",\"channel\":" + String(n.channel);
                json += ",\"auth\":\"" + NetworkMonitor::authName(n.auth) + "\"}";
            }
        }
        json += "]";
        server.send(200, "application/json", json);
    });

    server.on("/hash", HTTP_GET, [this]() {
        String input = server.arg("text");
        if (input.length() > 256) input = input.substring(0, 256);
        if (hashValue) *hashValue = sha256Hex(input);
        if (eventLogger) eventLogger->log("INFO", "SHA-256 computed via dashboard");
        server.sendHeader("Location", "/");
        server.send(303);
    });

    server.on("/api/logs", HTTP_GET, [this]() {
        server.send(200, "text/plain", eventLogger ? eventLogger->readAll() : "Logger unavailable");
    });

    server.on("/logs", HTTP_GET, [this]() {
        server.sendHeader("Content-Disposition", "attachment; filename=cyberlab-events.csv");
        server.send(200, "text/csv", eventLogger ? eventLogger->readAll() : "");
    });

    server.on("/api/status", HTTP_GET, [this]() {
        String json = "{\"uptime_ms\":" + String(millis()) +
                      ",\"free_heap\":" + String(ESP.getFreeHeap()) +
                      ",\"last_hash\":\"" + (hashValue ? *hashValue : "") + "\"}";
        server.send(200, "application/json", json);
    });

    server.onNotFound([this]() { server.send(404, "text/plain", "Not found"); });
}

void WebUI::handleClient() {
    server.handleClient();
}
