# Test plan for v0.1

- [ ] Firmware builds for `esp32-s3-devkitc-1`.
- [ ] Device boots and prints version to serial.
- [ ] LittleFS initializes and appends an event.
- [ ] `help`, `status`, `scan`, `hash`, and `logs` CLI commands work.
- [ ] SHA-256 of `abc` equals
      `ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad`.
- [ ] Setup AP is visible and requires the configured password.
- [ ] Dashboard loads at `http://192.168.4.1`.
- [ ] `/api/networks`, `/api/status`, and `/api/logs` return expected data.
- [ ] OLED initializes at configured I2C address (if installed).

Run tests only in an isolated lab and with networks you are authorized to inspect.
