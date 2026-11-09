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
    // Call only after Wi-Fi has been enabled: ESP32 hardware RNG then has
    // an entropy source suitable for generating the per-boot AP credential.
    static const char alphabet[] = "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz23456789";
    constexpr size_t passwordLength = 16;
    constexpr size_t alphabetLength = sizeof(alphabet) - 1;
    String password;
    password.reserve(passwordLength);
    for (size_t i = 0; i < passwordLength; ++i) {
        password += alphabet[esp_random() % alphabetLength];
    }
    return password;
}

static bool requestWifiScan(const char* source) {
    if (state.wifiScanRunning) return false;
    if (!networkMonitor.startScan()) {
        state.lastScanMs = millis();
        logger.log("ERROR", String(source) + ": unable to start Wi-Fi scan");
        return false;
    }
    state.wifiScanRunning = true;
    showScreen("Scanning Wi-Fi...");
    logger.log("INFO", String(source) + ": Wi-Fi scan started");
    return true;
}

static void pollWifiScan() {
    if (!state.wifiScanRunning) return;
    const int result = networkMonitor.pollScan(networks);
    if (result == NetworkMonitor::SCAN_RUNNING) return;

    state.wifiScanRunning = false;
    state.lastScanMs = millis();
    if (result == NetworkMonitor::SCAN_FAILED) {
        logger.log("ERROR", "Wi-Fi scan failed; retaining previous results");
        showScreen("Wi-Fi scan failed");
        return;
    }

    state.networkCount = result;
    logger.log("INFO", "Wi-Fi scan complete; networks=" + String(result));
    showScreen("Scan complete", "Networks: " + String(result), "Dashboard: see serial");
}

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\nESP32 CyberLab v0.1.0");

    Wire.begin(OLED_SDA, OLED_SCL);
    state.oledReady = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
    if (state.oledReady) showScreen("Booting...");

    if (!logger.begin()) Serial.println("WARNING: LittleFS unavailable; log persistence disabled");
    logger.log("INFO", "Device boot; filesystem mounted without auto-format");

    // Enable the radio before using esp_random() for the AP credential.
    WiFi.mode(WIFI_AP_STA);
    const char* apName = "CyberLab-Setup";
    String apPassword = generateApPassword();
    const bool apOk = WiFi.softAP(apName, apPassword.c_str());
    const IPAddress ip = WiFi.softAPIP();

    Serial.printf("Setup AP: %s\n", apName);
    if (apOk) {
        Serial.printf("Dashboard login: admin / %s\n", apPassword.c_str());
        Serial.print("Dashboard: http://");
        Serial.println(ip);
        Serial.println("Keep this credential private; it changes after reboot.");
        webUI.begin(&logger, &networks, &state.lastHash, &networkMonitor, &state, apPassword);
    } else {
        Serial.println("ERROR: Setup AP failed; dashboard is unavailable.");
        showScreen("AP startup failed", "Check power / radio");
    }

    networkMonitor.begin();
    if (apOk) requestWifiScan("Boot");
    showScreen(apOk ? "Ready" : "AP startup failed",
               apOk ? "AP: CyberLab-Setup" : "Dashboard offline",
               ip.toString());
}

void loop() {
    webUI.handleClient();
    pollWifiScan();

    if (!state.wifiScanRunning && millis() - state.lastScanMs >= WIFI_SCAN_INTERVAL_MS) {
        requestWifiScan("Scheduled");
    }

    // Serial CLI: type "scan", "status", "hash <text>", or "logs".
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();

        if (command == "scan") {
            if (!requestWifiScan("Serial")) Serial.println("Scan already running or could not start.");
        } else if (command == "status") {
            Serial.printf("Uptime: %lu ms | Free heap: %u | Networks: %d | Scan: %s\n",
                          millis(), ESP.getFreeHeap(), state.networkCount,
                          state.wifiScanRunning ? "running" : "idle");
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
