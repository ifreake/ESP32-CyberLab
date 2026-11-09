<!-- ESP32 CyberLab • Red / Black Edition | Documentation only; firmware unchanged. -->

<div align="center">

# 🔴⚫ ESP32 CyberLab

### **A hands-on lab for ESP32-S3, IoT security & embedded diagnostics**

[![PlatformIO](https://img.shields.io/badge/PlatformIO-IDE-orange?style=for-the-badge&logo=platformio&logoColor=white)](https://platformio.org/)
[![ESP32-S3](https://img.shields.io/badge/Hardware-ESP32--S3-111111?style=for-the-badge&logo=espressif&logoColor=red)](https://www.espressif.com/)
[![License: MIT](https://img.shields.io/badge/License-MIT-red?style=for-the-badge)](LICENSE)

**Explore. Build. Secure.**  
A compact educational platform for defensive IoT security, network diagnostics, cryptographic experiments, and embedded development.

</div>

---

> 🔴 **LAB RULE:** Use this project only with devices and networks you own or have explicit permission to assess.  
> ⚫ This is an educational lab tool — not a replacement for a professional Wi-Fi or network security assessment suite.

## 🧭 Contents

- [✨ Features](#-features)
- [🧭 Suggested Workflows](#-suggested-workflows)
- [🧰 Required Hardware](#-required-hardware)
- [🧩 Optional Hardware](#-optional-hardware)
- [🔌 OLED Wiring](#-oled-wiring-optional)
- [💻 Software Requirements](#-software-requirements)
- [🚀 Getting Started](#-getting-started)
- [📡 First Boot](#-first-boot)
- [⌨️ Serial Commands](#️-serial-commands)
- [🗂️ Project Layout](#️-project-layout)
- [🛡️ Security Notes](#️-security-notes)
- [🧯 Troubleshooting](#-troubleshooting)
- [🧪 Roadmap](#-roadmap)
- [📄 License](#-license)

## ✨ Features

| Feature | Description |
|---|---|
| 📡 Wi-Fi beacon scan | Displays SSID, RSSI, channel, and advertised authentication mode |
| 🔐 SHA-256 | Hashes text using the ESP-IDF mbedTLS implementation bundled with ESP32 Arduino |
| 📝 Event logging | Stores event logs in LittleFS |
| 🌐 Local dashboard | Provides a browser dashboard through a local setup access point |
| 🖥️ OLED status | Optional SSD1306 128×64 status display |
| ⌨️ Serial CLI | Includes `help`, `scan`, `status`, `hash <text>`, and `logs` |

<details>
<summary><strong>🔴 Lab scope & version</strong></summary>

- Documented firmware version: **v0.1.0**
- Target board: **ESP32-S3**
- Build system: **PlatformIO**
- Framework: **Arduino**
- Filesystem: **LittleFS**

</details>

## 🧭 Suggested Workflows

Try these workflows to explore the documented ESP32-CyberLab features. Use only your own devices and networks or systems you have permission to assess.

### 1. 🚀 First-Time Setup
1. Connect the ESP32-S3 DevKitC-1 to your computer with a USB data cable.
2. Open the repository in VS Code with the PlatformIO extension installed.
3. Build the firmware with `pio run`.
4. Upload it with `pio run -t upload`.
5. Open the serial monitor at **115200 baud** with `pio device monitor`.

### 2. 📡 Wi-Fi Network Discovery
1. Power on the ESP32-S3 and connect to the documented `CyberLab-Setup` access point.
2. Open the local dashboard at [http://192.168.4.1](http://192.168.4.1).
3. Use the documented `scan` serial command to run a Wi-Fi scan.
4. Review the reported SSID, signal strength (RSSI), channel, and advertised authentication mode.

### 3. 🔐 SHA-256 Experiment
1. Open the Serial Monitor at **115200 baud**.
2. Enter `help` to review the available commands.
3. Run `hash <text>` with a sample message, for example `hash Hello World`.
4. Record the resulting SHA-256 digest and compare it with a trusted SHA-256 implementation.

### 4. 📝 Event Log Review
1. Use the `logs` serial command to display available event logs.
2. Use `status` to inspect the device status.
3. Review the output when troubleshooting or validating a development change.

### 5. 🖥️ Optional OLED Setup
1. Connect an SSD1306 128×64 I²C display using the wiring table above.
2. Double-check the pinout for your exact ESP32-S3 board revision.
3. Power up the board and check the documented OLED status output.

### 6. 🧪 Development and Validation
1. Read [the architecture notes](docs/ARCHITECTURE.md) and [test plan](test/TEST_PLAN.md).
2. Make one small change at a time.
3. Build with `pio run`, then test the affected behavior on your own hardware.
4. Document the result and any limitations before marking a feature as complete.

> ⚠️ **Security note:** The setup access point uses a public development password, and the dashboard uses plain HTTP without login. Keep the device on an isolated lab setup; never expose it to the internet or an untrusted network. A Wi-Fi scan shows advertised network information and does not determine whether a network is vulnerable.

---

## 🧰 Required Hardware

These are the minimum items to build and connect the lab:

| Part | Quantity | Notes |
|---|---:|---|
| 🔴 ESP32-S3 development board | 1 | ESP32-S3-DevKitC-1 is the configured PlatformIO board |
| ⚫ USB data cable | 1 | Must support data, not charging only |
| 💻 Computer | 1 | Windows, Linux, or macOS; used to build and upload firmware |

**Board compatibility:** The current PlatformIO environment is configured for `esp32-s3-devkitc-1`. Other ESP32 variants are not guaranteed to work without changing the project configuration.

## 🧩 Optional Hardware

| Part | Purpose |
|---|---|
| 🖥️ SSD1306 OLED, 128×64, I²C | Displays device status |
| 🔌 Breadboard | Makes temporary wiring easier |
| 🧵 Dupont jumper wires | Connects the optional OLED |
| 🔍 USB-to-serial adapter | Optional, if your board's USB interface is unavailable |

## 🔌 OLED Wiring (Optional)

The documented default wiring is:

| OLED Pin | ESP32-S3 Pin |
|---|---|
| VCC | 3V3 |
| GND | GND |
| SDA | GPIO 8 |
| SCL | GPIO 9 |

⚠️ **Important:** Check the pinout of your exact ESP32-S3 board revision before connecting anything. Some boards use different I²C pins. Do not apply 5V logic signals directly to ESP32 GPIO pins.

## 💻 Software Requirements

Install the following:

1. **[Visual Studio Code](https://code.visualstudio.com/)**
2. **[PlatformIO IDE extension](https://platformio.org/install/ide?install=vscode)**
3. A USB driver for your board, if your operating system does not recognize its USB-to-serial interface.

The repository includes a `platformio.ini` configured for the ESP32-S3 DevKitC-1, Arduino framework, 115200 baud serial monitor, and LittleFS.

## 🚀 Getting Started

### 1. Get the project

    git clone https://github.com/ifreake/ESP32-CyberLab.git
    cd ESP32-CyberLab

Or download the repository as a ZIP file from GitHub and extract it.

### 2. Open it in VS Code

- Launch Visual Studio Code.
- Choose **File → Open Folder**.
- Select the `ESP32-CyberLab` folder.
- Make sure the PlatformIO extension is installed and initialized.

### 3. Connect the board

Connect the ESP32-S3 to your computer with a USB data cable. If no serial port appears, try another cable and install the USB-to-serial driver required by your board.

### 4. Build the firmware

Open the VS Code terminal in the project folder and run:

    pio run

This compiles the project. Resolve any build errors before uploading.

### 5. Upload to the ESP32-S3

    pio run -t upload

If PlatformIO cannot select the correct serial port automatically, configure the upload port in your local PlatformIO environment.

### 6. Open the Serial Monitor

    pio device monitor

The configured monitor speed is **115200 baud**.

## 📡 First Boot

After startup, the firmware is documented to create a local setup access point:

| Setting | Value |
|---|---|
| Wi-Fi name (SSID) | `CyberLab-Setup` |
| Development password | `LabSetup-2026` |
| Dashboard address | [http://192.168.4.1](http://192.168.4.1) |

1. Power on the ESP32-S3.
2. On your phone or computer, connect to `CyberLab-Setup`.
3. Open a browser and visit `http://192.168.4.1`.
4. Keep this setup network isolated and use it only in your own lab.

🔴 **Security warning:** The listed password is a development default and is public because it appears in this documentation. Do not reuse it for a real deployment. The dashboard currently uses plain HTTP and has no user login; do not expose it to the internet or an untrusted LAN. Changing the documentation does not change the password stored in firmware.

## ⌨️ Serial Commands

Use the Serial Monitor at **115200 baud**.

| Command | Purpose |
|---|---|
| `help` | Shows available commands |
| `scan` | Runs the Wi-Fi scan |
| `status` | Displays device status |
| `hash <text>` | Calculates a SHA-256 hash for the supplied text |
| `logs` | Displays available event logs |

Example:

    hash Hello World

## 🗂️ Project Layout

```text
ESP32-CyberLab/
├── .gitignore
├── LICENSE
├── README.md
├── platformio.ini
├── docs/
│   └── ARCHITECTURE.md
├── hardware/
│   └── BOM.md
├── include/
│   ├── app_state.h
│   ├── config.h
│   ├── crypto_lab.h
│   ├── logger.h
│   ├── network_monitor.h
│   └── web_ui.h
├── src/
│   ├── crypto_lab.cpp
│   ├── logger.cpp
│   ├── main.cpp
│   ├── network_monitor.cpp
│   └── web_ui.cpp
└── test/
    └── TEST_PLAN.md
```

| Folder / file | Purpose |
|---|---|
| `include/` | Shared headers and configuration |
| `src/` | Firmware implementation files |
| `docs/` | Architecture and development documentation |
| `hardware/` | Bill of materials and wiring notes |
| `test/` | Test plan and future automated tests |
| `platformio.ini` | PlatformIO board, framework, build, and monitor settings |

## 🛡️ Security Notes

- 🔴 Only test on hardware and networks you own or are authorized to assess.
- ⚫ The setup access point uses a development-default password; do not treat it as production security.
- 🌐 The dashboard currently has no authentication and uses unencrypted HTTP.
- 📡 A Wi-Fi scan reports advertised network properties. It does **not** prove that a network is secure or vulnerable.
- 🔐 Secure Boot and Flash Encryption are not enabled by this starter firmware. Configure and test them separately on a spare development board.
- ⚠️ eFuse changes can be irreversible. Read the official Espressif documentation carefully before enabling security features.
- 🗝️ Never commit private keys, production credentials, or personal network data.

## 🧯 Troubleshooting

| Problem | Things to check |
|---|---|
| Board is not detected | Try a known-good USB data cable, another USB port, and the correct USB-to-serial driver |
| Upload fails | Confirm the selected serial port; close other apps using that port; retry with the board's BOOT procedure if required |
| Serial output is unreadable | Set the monitor speed to **115200 baud** |
| Dashboard does not open | Confirm that you joined `CyberLab-Setup` and entered `http://192.168.4.1` |
| OLED stays blank | Check VCC, GND, SDA, SCL, board pinout, and the display's I²C compatibility |

## 🧪 Roadmap

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

## 📚 Useful Resources

- [ESP32-S3 Documentation — Espressif](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/)
- [PlatformIO Documentation](https://docs.platformio.org/)
- [Arduino-ESP32 Documentation](https://docs.espressif.com/projects/arduino-esp32/en/latest/)
- [LittleFS Documentation](https://github.com/littlefs-project/littlefs)

## 📄 License

This project is licensed under the **MIT License**. See [LICENSE](LICENSE).

---

<div align="center">

### 🔴⚫ Built for learning. Designed for experimentation. Use responsibly. ⚫🔴

**ESP32 CyberLab · v0.1.0**

</div>
