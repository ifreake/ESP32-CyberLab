# Bill of Materials

## Minimum build
| Part | Qty | Notes |
|---|---:|---|
| ESP32-S3 DevKitC-1 or compatible | 1 | Prefer a module with sufficient flash; verify exact SKU |
| USB data cable | 1 | Must support data, not charge-only |

## Optional user interface
| Part | Qty | Notes |
|---|---:|---|
| SSD1306 128x64 I2C OLED | 1 | 3.3V logic; address commonly 0x3C |
| Tactile push buttons | 3-4 | For a future physical menu |
| Rotary encoder | 1 | Optional navigation input |
| Breadboard and jumper wires | 1 set | Prototyping |

## Future security expansion
| Part | Qty | Notes |
|---|---:|---|
| ATECC608A breakout | 1 | Secure element; check library and provisioning workflow |
| DS3231 RTC | 1 | Battery-backed timestamp source |
| microSD SPI module | 1 | Optional exportable logs; verify 3.3V compatibility |

Do not buy all optional parts for v0.1. The core firmware runs without an OLED.
Check your exact ESP32-S3 board pinout and voltage requirements before wiring.
