# ESP32 CyberLab

An open-source, ESP32-S3-based learning platform for defensive IoT security,
network diagnostics, cryptographic experiments, and embedded development.

> **Scope:** Use this project only on networks and devices you own or have
> explicit permission to assess. This is an educational lab tool, not a
> replacement for a professional Wi-Fi or network security assessment suite.

## Features in v0.1.0

- Wi-Fi beacon scan: SSID, RSSI, channel, and advertised authentication mode.
- SHA-256 hashing using the ESP-IDF mbedTLS implementation bundled with ESP32 Arduino.
- Event logging to LittleFS.
- Local setup access point and browser dashboard.
- OLED status screen (SSD1306 128x64, optional).
- Serial CLI: `help`, `scan`, `status`, `hash <text>`, `logs`.

## Hardware

Required:
- ESP32-S3 development board (for example ESP32-S3-DevKitC-1).
- USB data cable.

Optional:
- SSD1306 128x64 I2C OLED.
- Breadboard and jumper wires.

### OLED wiring

Default pins are configurable in `include/config.h`.

| OLED | ESP32-S3 |
|---|---|
| VCC | 3V3 |
| GND | GND |
| SDA | GPIO 8 |
| SCL | GPIO 9 |

Check the pinout of your exact board revision before wiring. Some boards use
different convenient I2C pins. Do not connect 5V logic signals directly to ESP32 GPIO.

## Build with PlatformIO

1. Install Visual Studio Code and the PlatformIO IDE extension.
2. Open this repository as a PlatformIO project.
3. Connect the ESP32-S3 over USB.
4. Build, upload, and open the serial monitor:

```bash
pio run
pio run -t upload
pio device monitor
```

If your board uses a different USB-to-UART arrangement, select the correct
upload port and USB mode for that board.

## First boot

The firmware starts a local setup AP:

- SSID: `CyberLab-Setup`
- Password: `LabSetup-2026`
- Dashboard: `http://192.168.4.1`

**Change the default AP password before using the device outside an isolated lab.**
The v0.1 dashboard uses HTTP and is intended only for a trusted, isolated setup
network. Do not expose it to the internet or an untrusted LAN.

## Serial commands

```text
help
scan
status
hash Hello World
logs
```

## Repository layout

```text
include/       Shared headers and configuration
src/           Firmware source modules
docs/          Architecture and development notes
hardware/      Wiring and bill of materials
test/          Test plan and future automated tests
```

## Roadmap

- [x] Initial PlatformIO project structure
- [x] Wi-Fi scan and local dashboard
- [x] SHA-256 utility
- [x] LittleFS event logger
- [x] OLED status output
- [ ] MQTT over TLS with certificate validation
- [ ] Device identity and mutual authentication
- [ ] HMAC-based message authentication
- [ ] Secure OTA update flow
- [ ] ESP32-S3 Secure Boot and Flash Encryption documentation
- [ ] Automated unit and hardware-in-the-loop tests

## Security notes

- The setup AP password is a development default; change it before deployment.
- The dashboard currently has no user login and uses plain HTTP.
- The dashboard is designed for a directly connected, isolated setup AP.
- A Wi-Fi scan reports advertised network properties; it does not prove that a
  network is secure or vulnerable.
- Secure Boot and Flash Encryption are not enabled by this starter firmware.
  Configure and test them separately on a spare development board. eFuse changes
  can be irreversible.
- Never commit private keys, production credentials, or personal network data.

## License

MIT. See [LICENSE](LICENSE).
