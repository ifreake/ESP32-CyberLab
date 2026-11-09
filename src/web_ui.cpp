#include "web_ui.h"
#include "crypto_lab.h"
#include <WiFi.h>
#include <esp_system.h>

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
                } else output += static_cast<char>(c);
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
                  const String& password) {
    eventLogger = logger;
    scanResults = networks;
    hashValue = lastHash;
    networkMonitor = monitor;
    appState = state;
    accessPassword = password; // Own a copy; never retain a pointer to setup()'s local String.
    static const char hex[] = "0123456789abcdef";
    csrfToken = "";
    csrfToken.reserve(32);
    for (int i = 0; i < 16; ++i) {
        const uint32_t value = esp_random();
        for (int shift = 28; shift >= 0; shift -= 4) csrfToken += hex[(value >> shift) & 0x0f];
    }
    const char* headers[] = {"X-CyberLab-Request"};
    server.collectHeaders(headers, 1);
    routes();
    server.begin();
}

bool WebUI::authorize() {
    if (accessPassword.length() && server.authenticate("admin", accessPassword.c_str())) return true;
    server.requestAuthentication(BASIC_AUTH, "ESP32 CyberLab", "Valid dashboard credentials required");
    return false;
}

bool WebUI::authorizeMutation() {
    if (!authorize()) return false;
    // A per-boot secret in a non-simple request header blocks cross-origin HTML forms.
    // Do not enable permissive CORS on this local-only dashboard.
    if (!server.hasHeader("X-CyberLab-Request") ||
        server.header("X-CyberLab-Request") != csrfToken) {
        server.send(403, "text/plain; charset=utf-8", "CSRF validation failed");
        return false;
    }
    return true;
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
small{color:#9fb0c5}input{padding:9px;border-radius:6px}
</style></head><body><h1>ESP32 CyberLab</h1><small>Local lab dashboard · v0.1.0</small>
<div class="card"><h2>Wi-Fi scan results</h2><button id="scan">Run scan</button><div id="networks">Loading…</div></div>
<div class="card"><h2>SHA-256</h2><form id="hashForm"><input id="hashInput" maxlength="256" placeholder="Text to hash" required><button>Hash</button></form><p id="hash"></p></div>
<div class="card"><h2>Recent events</h2><pre id="logs">Loading…</pre><a href="/logs">Download log</a></div>
<script>
const csrfToken='__CSRF_TOKEN__';
async function mutate(path,body){
 const r=await fetch(path,{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded','X-CyberLab-Request':csrfToken},body});
 if(!r.ok)throw Error(await r.text()); return r;
}
function cell(row,value){const td=document.createElement('td');td.textContent=String(value);row.appendChild(td);}
async function refresh(){
 try{const r=await fetch('/api/networks');if(!r.ok)throw Error();const items=await r.json();
 const table=document.createElement('table'),head=document.createElement('tr');
 ['SSID','RSSI','Channel','Security'].forEach(v=>cell(head,v));table.appendChild(head);
 items.forEach(n=>{const row=document.createElement('tr');cell(row,n.ssid);cell(row,n.rssi+' dBm');cell(row,n.channel);cell(row,n.auth);table.appendChild(row);});
 document.getElementById('networks').replaceChildren(table);
 }catch(e){document.getElementById('networks').textContent='Unable to load scan results';}
 try{const r=await fetch('/api/logs');if(!r.ok)throw Error();document.getElementById('logs').textContent=(await r.text()).slice(-1800);}catch(e){document.getElementById('logs').textContent='Unable to load logs';}
 try{const r=await fetch('/api/status');if(r.ok){const s=await r.json();document.getElementById('hash').textContent=s.last_hash?'SHA-256: '+s.last_hash:'No hash generated yet';}}catch(e){}
}
document.getElementById('scan').addEventListener('click',async()=>{
 try{await mutate('/scan','');document.getElementById('networks').textContent='Scan started…';}
 catch(e){document.getElementById('networks').textContent='Scan request failed: '+e.message;}
});
document.getElementById('hashForm').addEventListener('submit',async e=>{
 e.preventDefault();try{await mutate('/hash','text='+encodeURIComponent(document.getElementById('hashInput').value));await refresh();}
 catch(err){document.getElementById('hash').textContent='Hash request failed: '+err.message;}
});
refresh();setInterval(refresh,5000);
</script></body></html>)HTML";
        page.replace("__CSRF_TOKEN__", csrfToken);
        server.sendHeader("Cache-Control", "no-store");
        server.send(200, "text/html; charset=utf-8", page);
    });

    server.on("/scan", HTTP_POST, [this]() {
        if (!authorizeMutation()) return;
        if (!networkMonitor || !scanResults || !appState) {
            server.send(503, "text/plain", "Scanner unavailable"); return;
        }
        if (appState->wifiScanRunning) {
            server.send(409, "text/plain", "Scan already running"); return;
        }
        if (!networkMonitor->startScan()) {
            server.send(503, "text/plain", "Unable to start Wi-Fi scan"); return;
        }
        appState->wifiScanRunning = true;
        if (eventLogger) eventLogger->log("INFO", "Dashboard Wi-Fi scan started");
        server.send(202, "application/json", "{\"status\":\"started\"}");
    });

    server.on("/hash", HTTP_POST, [this]() {
        if (!authorizeMutation()) return;
        String input = server.arg("text");
        if (input.length() > 256) {
            server.send(400, "text/plain", "Input too long"); return;
        }
        if (hashValue) *hashValue = sha256Hex(input);
        if (eventLogger) eventLogger->log("INFO", "SHA-256 computed via dashboard");
        server.send(200, "application/json", "{\"status\":\"ok\"}");
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
        server.sendHeader("Cache-Control", "no-store");
        server.send(200, "application/json; charset=utf-8", json);
    });

    server.on("/api/logs", HTTP_GET, [this]() {
        if (!authorize()) return;
        server.sendHeader("Cache-Control", "no-store");
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
                      ",\"wifi_scan_running\":" + String(appState && appState->wifiScanRunning ? "true" : "false") +
                      ",\"last_hash\":\"" + jsonEscape(hashValue ? *hashValue : "") + "\"}";
        server.sendHeader("Cache-Control", "no-store");
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
