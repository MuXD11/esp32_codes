# ESP32 TTGO T-Beam LoRa Telemetry Platform

A bidirectional LoRa telemetry system built on the **LilyGo TTGO T-Beam V1.0** platform, designed as the communications and GNSS backbone for an embedded flight payload. The project pairs a flight-side transmitter with a ground-side receiver.

The platform is intentionally focused on **communications, GNSS handling, and LoRa link management**. The sensor acquisition and data analysis layers are kept minimal so that the protocol, the radio configuration, and the dynamic GPS behavior can be developed and validated in isolation. Real sensor integration is straightforward to layer on top once the platform is stable.

This project was developed as part of my thesis work for the Universidad de Alcalá, in the 2024/25 academic year.

> ⚠️ **Status: actively evolving.** This codebase is under active development and several subsystems are partially implemented. See the per-segment notes below for what is working, what is stubbed, and what is in progress.

---

## Hardware

All three subprojects target the **LilyGo TTGO T-Beam V1.0** (also known as Rev1), which integrates:

- **ESP32** dual-core MCU (Xtensa LX6, 240 MHz)
- **Semtech SX1276** LoRa transceiver, configurable for 433/868/915 MHz
- **u-blox NEO-6M** GPS module
- **4 MB SPI flash** (the dominant variant; some newer boards ship with 16 MB)
- **AXP192** power management IC controlling GPS and LoRa power rails
- 18650 lithium battery holder for portable operation

The flight code targets the 868 MHz EU ISM band by default. Pin assignments in each subproject's `configuration.h` are set for the V1.0 board layout (SPI on GPIO 5/19/27/18, LoRa reset on GPIO 14 or 23 depending on revision, GPS UART on GPIO 34/12).

### About the NEO-6M GPS

The [u-blox NEO-6M datasheet](<https://content.u-blox.com/sites/default/files/products/documents/NEO-6_DataSheet_(GPS.G6-HW-09005).pdf>) specifies the following operational limits:

| Parameter                         | Value    |
| --------------------------------- | -------- |
| Max dynamics                      | 4 g      |
| Max altitude (hardware)           | 50,000 m |
| Max velocity                      | 500 m/s  |
| Max navigation update rate        | 5 Hz     |
| Tracking & navigation sensitivity | -161 dBm |
| Cold start TTFF                   | 27 s     |
| Hot start TTFF                    | 1 s      |

The module supports several **dynamic platform models** configurable via the UBX-CFG-NAV5 message. Each model applies internal sanity checks against expected altitude, velocity, and vertical-velocity limits; if a check fails, the position solution is invalidated. The two models relevant to this project are:

- **Portable** (`dynModel = 0`, default): designed for ground-level use, with an altitude sanity-check limit of **9,000 m**. The receiver will refuse to report fixes above this altitude.
- **Airborne <1g** (`dynModel = 6`): designed for low-acceleration airborne use such as balloon flights. Allows altitudes up to **50,000 m** and horizontal velocities up to **100 m/s**.

For the full table of supported models and their internal limits, refer to the u-blox 6 [Receiver Description and Protocol Specification](<https://content.u-blox.com/sites/default/files/products/documents/u-blox6_ReceiverDescrProtSpec_(GPS.G6-SW-10018)_Public.pdf>).

---

## Repository Structure

```
.
├── flight_code/              Flight-side software (transmitter)
├── ground_segment_code/      Ground-station software (LoRa-to-cloud bridge)
└── plain_lora_receiver/      Standalone LoRa packet logger
```

Each folder is a self-contained PlatformIO project that can be built and uploaded independently.

---

## Subproject: Flight Segment (`flight_code/`)

The flight-side firmware runs on the airborne T-Beam. It samples telemetry, transmits it over LoRa at a configurable cadence, and listens for inbound telecommands during quiet windows.

### Code organization

| File                     | Responsibility                                                                                        |
| ------------------------ | ----------------------------------------------------------------------------------------------------- |
| `main.cpp`               | Setup, main 10-second loop, active-listening window, test injection blocks                            |
| `configuration.h`        | All tunable constants: operation mode, LoRa defaults, GPS pins, debug verbosity, test-injection flags |
| `gps.h/.cpp`             | GPS UART, UBX protocol (CFG-NAV5 poll/set), altitude-driven dynamic-mode logic                        |
| `clora.h/.cpp`           | LoRa SPI bus configuration and low-level register reads                                               |
| `data.h/.cpp`            | Telemetry and telecommand struct definitions, sensor sampling dispatcher                              |
| `tc_handling.h/.cpp`     | Telecommand dispatcher and individual command handlers                                                |
| `flash_managment.h/.cpp` | LittleFS init and generic byte-level read/write helpers                                               |

### Telecommands implemented

| ID  | Action                                                                |
| --- | --------------------------------------------------------------------- |
| 1   | System reset                                                          |
| 2   | Change loop period (with bounds validation)                           |
| 10  | Reconfigure LoRa spreading factor (validated against SX127x range)    |
| 11  | Reconfigure LoRa bandwidth (validated against the 10 discrete values) |
| 12  | Reconfigure LoRa coding rate                                          |
| 13  | Change LoRa TX power                                                  |

The dispatcher takes the received telecommand by `const` reference and routes by `TC_Action_ID`. Each handler validates its payload before applying it, returning a status flag so the dispatcher can log failures. This telecommand table is still **under development**, in an effort to implement more and more sophisticated functionalities

### GPS dynamic-mode workaround

The NEO-6M ships in **Portable** mode by default (`dynModel = 0`), which imposes an internal altitude sanity-check limit of **9,000 m**. For a balloon flight, this is well within the expected cruise altitude, meaning the receiver would stop producing fixes early in the climb. The fix is to switch the module into **Airborne <1g** mode (`dynModel = 6`), which extends the altitude limit to 50,000 m and the horizontal velocity limit to 100 m/s.

Because the TinyGPS++ library used in this project does not expose UBX configuration helpers, the necessary UBX-CFG-NAV5 frame is **hand-encoded as a byte sequence** and sent directly over UART. The frame to activate Airborne <1g mode is:

```
UBX-CFG-NAV5 (Activate Airborne <1g)
0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x06, 0x03,
0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x05, 0x00,
0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x16, 0xDC
```

The configuration is applied in RAM and remains active while the module is powered. It is **not persisted across power cycles** — if the GPS resets in flight, it reverts to Portable mode.

Rather than setting Airborne mode once at boot, the firmware implements a **closed-loop self-correcting approach**:

1. On every main-loop iteration, a **UBX-CFG-NAV5 poll** (`0xB5 0x62 0x06 0x24 0x00 0x00 ...`) is sent to the GPS.
2. The response is parsed in `leerRespuestaNAV5()`, which extracts the current `dynModel` byte and stores it in a control variable.
3. `alt_proc()` examines the current altitude and the reported `dynModel`:
   - Below ~12 km: Portable mode is acceptable (`dynModel = 0`)
   - Above ~12 km: Airborne <1g mode is required (`dynModel = 6`)
   - If there is a mismatch, the corresponding UBX-CFG-NAV5 _set_ frame is dispatched to switch the model
4. The next poll confirms the change took effect

This automatic switching ensures the GPS recovers correctly if the module power-cycles or resets mid-flight. The polling cost is negligible (a few bytes per iteration), and the system stays self-correcting without any operator intervention.

A test-injection block in `main.cpp` (gated by `ENABLE_ALTITUDE_TEST_INJECTION` in `configuration.h`) forces altitude values into the 14 km range between loop counters 10 and 20, allowing the dynamic-mode switch to be exercised on the bench without an actual flight.

### Communications and listening

The flight segment uses a **time-multiplexed transmit/receive pattern** rather than interrupt-driven RX:

- The first ~1–2 seconds of each loop iteration handle telemetry sampling, GPS dynamic-mode checks, and LoRa transmission.
- The remainder of the configurable period (default 10 seconds) is spent in an **active-listening window** polling `LoRa.parsePacket()` every 2 seconds.
- Incoming packets are validated against the expected `Data_TC_1` size, copied into the global telecommand buffer, and flagged for dispatch at the top of the next loop iteration.

This bidirectional capability — receiving telecommands in-flight to dynamically reconfigure the system — has been **partially tested and is still under active development**. Specifically:

- The reception path works on the bench with hand-crafted packets from the ground station.
- Several telecommand handlers (system reset, LoRa parameter changes, TX power) are implemented and validated.
- Integration testing of the full ground→flight loop under realistic radio conditions (range, attitude variations, antenna polarization) is not yet complete.
- The latency between TC transmission and reception can be up to 2 seconds due to the polling interval; an interrupt-driven RX refactor is planned but not yet started.

### Focus and scope

Sensor acquisition and data analysis are **explicitly out of scope for the current development**. The `gather_sensor_data()` function currently operates in two modes:

- `CURRENT_MODE_DEBUG = 1`: returns simulated values for temperature, pressure, humidity, and barometric altitude, with real GPS lat/lon/HDOP.
- `CURRENT_MODE_DEBUG_SENSOR_ON = 1`: branch reserved for real BME280 / external probe sampling (driver wired up but not the primary focus).

The intent is that the communications, GNSS, and LoRa management layers are robust and well-tested _first_, after which sensor drivers can be layered in without requiring changes to the platform.

### Known limitations

- UBX checksum validation on received CFG-NAV5 responses is not yet implemented: a corrupted frame could be accepted as valid dynamic-mode status. Mitigation: the UBX header sync bytes provide a coarse integrity check.
- The flash subsystem provides generic `init / write / read` byte-level operations; no telemetry logging policy is wired into the main loop yet.

---

## Subproject: Ground Segment (`ground_segment_code/`)

The ground-side firmware runs on a stationary T-Beam connected to mains power and Wi-Fi. It acts as a **LoRa-to-cloud bridge**: receiving telemetry from the flight segment, optionally validating and logging it locally, then forwarding it to a remote HTTPS server. It also polls the same server for pending telecommands and forwards them back to the flight segment over LoRa.

### Code organization

The ground segment is split into focused subsystems:

| File               | Responsibility                                                       |
| ------------------ | -------------------------------------------------------------------- |
| `main.cpp`         | Setup and orchestration loop                                         |
| `configuration.h`  | Pin definitions, WiFi credentials, LoRa parameters, OLED config      |
| `data.h/.cpp`      | Shared telemetry and telecommand struct definitions                  |
| `lora_rx.h/.cpp`   | LoRa SPI bus, init, packet reception, packet transmission            |
| `server.h/.cpp`    | WiFi management, HTTPS POST of telemetry, HTTPS GET for telecommands |
| `flash_log.h/.cpp` | LittleFS log file for received packet sequence numbers               |
| `utils.h/.cpp`     | OLED display wrapper, small helpers                                  |

### Behavior

1. On boot: brings up LoRa, OLED, WiFi, and LittleFS.
2. In the main loop:
   - Polls LoRa for incoming telemetry. On reception:
     - Overwrites GPS coordinates with a fixed location (current ground-test behavior; remove for real-flight deployment).
     - Updates the OLED with the latest sequence number.
     - Appends the sequence number to a local LittleFS log.
     - Forwards the full payload to the cloud server as JSON over HTTPS.
   - When idle, optionally polls the server for pending telecommands (currently pending full integration testing).

### Server backend

The HTTPS endpoints point to a separate Flask backend hosted on Render, which provides the operator interface for visualizing telemetry and dispatching telecommands.

**Server repository:** [https://github.com/MuXD11/IOT_app](https://github.com/MuXD11/IOT_app)

The README in that repository documents the API contract (`/api/datos` for telemetry POSTs, `/api/comando` for telecommand polling) and the JSON schema expected by both endpoints.

---

## Subproject: Plain LoRa Receiver (`plain_lora_receiver/`)

A minimal third subproject that **only receives LoRa packets and logs them**. Useful for:

- Validating the radio link in isolation when debugging RX issues on the ground segment.
- Capturing packet traces for offline analysis.
- Verifying that the flight segment's transmission cadence and payload format match expectations without involving the full ground-station stack.

The receiver writes received packets to LittleFS and prints them to the serial console. It is intentionally simple and shares the LoRa configuration constants with the ground segment so its behavior matches a real receiver.

---

## Build and Deployment

### PlatformIO

All three subprojects are built using **PlatformIO**. Each folder contains its own `platformio.ini` and can be opened independently in VS Code with the PlatformIO extension, or built from the command line:

```bash
cd flight_code/
pio run                    # build
pio run --target upload    # build + flash over USB
pio device monitor         # open serial monitor at 115200 baud
```

### Board configuration

In each `platformio.ini`:

```ini
[env:ttgo-t-beam]
platform = espressif32
board = ttgo-lora32-v1
framework = arduino
monitor_speed = 115200
upload_speed = 921600
```

Note that the PlatformIO board ID is a close match rather than an exact one for the T-Beam V1.0 — the pin definitions in `configuration.h` are what actually matter, and they are set explicitly.

### Partition scheme

By default, a 4 MB ESP32 flash gives only ~190 KB to LittleFS, which is fine for the current sequence-number log but tight if the log file grows. For larger on-board logs, switch to a no-OTA partition scheme in `platformio.ini`:

```ini
board_build.partitions = min_spiffs.csv
```

This reclaims the unused OTA slot for the filesystem (~2 MB available).

### Dependencies

Each subproject's `platformio.ini` declares its dependencies via `lib_deps`. The main libraries used across the projects are:

- **Arduino-LoRa** (sandeepmistry/LoRa) — raw LoRa peer-to-peer, not LoRaWAN
- **TinyGPS++** — NMEA parsing for the GPS module
- **LittleFS** — included in the ESP32 Arduino core (≥2.0.0), no extra install
- **Adafruit BME280 + Unified Sensor** — for the BME280 driver (flight segment)
- **Adafruit GFX + SSD1306** — for the OLED display (ground segment)
- **ArduinoJson** — for telecommand JSON parsing (ground segment)

---

## Communications Protocol

### Frame format (LoRa)

Both telemetry (flight → ground) and telecommands (ground → flight) are sent as **raw binary structs** over LoRa, with no application-layer framing:

- **Telemetry** (`Data` struct): ~42 bytes, fixed layout, sent every `loop_period_ms` (default 10 s).
- **Telecommand** (`Data_TC_1` struct): 6 bytes, fixed layout, sent on demand from the ground server.

The receiver on each side validates the LoRa packet size matches `sizeof(StructType)` before copying; packets of unexpected sizes are drained from the radio buffer and discarded.

This approach is simple and efficient but assumes both ends agree on struct layout, padding, and endianness. As long as both flight and ground run on ESP32 with the same compiler, this is safe. If you ever talk to a different MCU, switch to explicit serialization.

---

## License

This project is released under the **MIT License**:

```
MIT License

Copyright (c) 2026 Carlos Ene

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

For convenience, this license text is also typically saved in a separate `LICENSE` file at the repository root.

The MIT License is one of the most permissive open-source licenses: it allows anyone to use, modify, distribute, or even commercially exploit your code, with the only requirement being that the original copyright notice is retained. It is a sensible default for academic and personal projects that the author wants to share without restrictions, and it is widely used and recognized in the embedded and IoT communities.

---
