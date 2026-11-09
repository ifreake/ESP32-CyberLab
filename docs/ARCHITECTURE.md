# Architecture

The firmware is intentionally split into small modules:

- `NetworkMonitor`: scans nearby Wi-Fi beacons and returns advertised metadata.
- `EventLogger`: writes timestamped events to LittleFS.
- `Crypto Lab`: exposes SHA-256 as a small reusable primitive.
- `WebUI`: serves a local dashboard and read-only JSON/log endpoints.
- `main.cpp`: owns startup and the cooperative main loop.

The current version uses Arduino framework on top of ESP-IDF. This is a
pragmatic PlatformIO starter. Security-sensitive production features should be
implemented with explicit threat modeling, secure key provisioning, and tests.
