#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <esp_system.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <vector>

#include "config.h"
#include "app_state.h"
#include "logger.h"
#include "crypto_lab.h"
#include "network_monitor.h"
#include "web_ui.h"

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);
EventLogger logger;
NetworkMonitor networkMonitor;
WebUI webUI;
AppState state;
std::vector<NetworkInfo> networks;

static void showScreen(const String& line1, const String& line2 = "", const String& line3 = "") {
    if (!state.oledReady) return;
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("ESP32 CYBERLAB");
    display.println("---------------------");
    display.println(line1);
    if (line2.length()) display.println(line2);
    if (line3.length()) display.println(line3);
    display.display();
}

static String generateApPassword() {
    // A fresh per-boot credential avoids shipping a shared, public default.
    static const char alphabet[] = "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz23456789";
    constexpr size_t passwordLength = 16;
    String password;
    password.reserve(passwordLength);
    const size_t alphabetLength = sizeof(alphabet) - 1;
    for (size_t i = 0; i < passwordLength; ++i) {
        password += alphabet[esp_random() % alphabetLength];
    }
    return password;
}

static void runScan() {
    showScreen("Scanning Wi-Fi...");
    state.wifiScanRunning = true;
    int found = networkMonitor.scan(networks);
    state.networkCount = found > 0 ? found : 0;
    state.lastScanMs = millis();
    state.wifiScanRunning = false;
    logger.log(found >= 0 ? "INFO" : "ERROR", "Wi-Fi scan finished; networks=" + String(found));
    showScreen("Scan complete", "Networks: " + String(state.networkCount), "Dashboard: see serial");
}

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\nESP32 CyberLab v0.1.0");

    Wire.begin(OLED_SDA, OLED_SCL);
    state.oledReady = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
    if (state.oledReady) showScreen("Booting...");

    if (!logger.begin()) Serial.println("WARNING: LittleFS init failed");
    logger.log("INFO", "Device boot");

    networkMonitor.begin();

    // The setup AP is local-only. Its random password changes on every reboot.
    const char* apName = "CyberLab-Setup";
    String apPassword = generateApPassword();
    WiFi.mode(WIFI_AP_STA);
    bool apOk = WiFi.softAP(apName, apPassword.c_str());
    IPAddress ip = WiFi.softAPIP();

    Serial.printf("Setup AP: %s\n", apName);
    Serial.printf("Dashboard login: admin / %s\n", apPassword.c_str());
    Serial.print("Dashboard: http://");
    Serial.println(ip);
    Serial.println("Keep this credential private; it changes after reboot.");

    logger.log(apOk ? "INFO" : "ERROR", "Setup access point initialization");
    webUI.begin(&logger, &networks, &state.lastHash, &networkMonitor, &state, &apPassword);

    runScan();
    showScreen("Ready", "AP: CyberLab-Setup", ip.toString());
}

void loop() {
    webUI.handleClient();

    if (!state.wifiScanRunning && millis() - state.lastScanMs >= WIFI_SCAN_INTERVAL_MS) {
        runScan();
    }

    // Serial CLI: type "scan", "status", "hash <text>", or "logs".
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();

        if (command == "scan") {
            runScan();
        } else if (command == "status") {
            Serial.printf("Uptime: %lu ms | Free heap: %u | Networks: %d\n",
                          millis(), ESP.getFreeHeap(), state.networkCount);
        } else if (command.startsWith("hash ")) {
            String value = command.substring(5);
            state.lastHash = sha256Hex(value);
            Serial.println("SHA256: " + state.lastHash);
            logger.log("INFO", "SHA-256 computed via serial CLI");
        } else if (command == "logs") {
            Serial.println(logger.readAll());
        } else if (command == "help") {
            Serial.println("Commands: help, scan, status, hash <text>, logs");
        } else if (command.length()) {
            Serial.println("Unknown command. Type help.");
        }
    }

    delay(5);
}
