# Changelog

[🇩🇪 Deutsch](CHANGELOG.md) | 🇬🇧 English | [🇫🇷 Français](CHANGELOG.fr.md)

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

---

## Compatibility Status

### Summary

This release restores compatibility with ESPHome 2025.8 through 2026.4.x.
The primary driver was the removal of `get_uart_event_queue()` from the upstream
`IDFUARTComponent` in ESPHome 2025.8, which broke LIN-bus BREAK detection on
ESP-IDF builds. Additional breaking changes in ESP-IDF 5.x (ESP32 toolchain) and
ESPHome 2026.x API changes were also resolved.

Tested against:
- ESPHome **2026.4.1** — ESP-IDF ✅
- ESPHome **2026.4.0** — ESP-IDF ✅
- ESPHome **2026.3.3** — ESP-IDF ✅
- ESPHome **2026.3.2** — ESP-IDF ✅
- ESPHome **2026.3.1** — ESP-IDF ✅
- ESPHome **2026.3.0** — ESP-IDF ✅

---


## [1.0.14] — 2026-04-21 — Truma Cooler C(XX) integration

### Added
- Support for the Truma Cooler C(XX) series via BLE (active connection via `ble_client`)
- New `truma_cooler` component under `components/truma_cooler/` (alongside `truma_inetbox` and `uart`)
- Entities: Climate (setpoint −22 °C to +10 °C), interior temperature, ambient temperature, compressor status, turbo switch, device status, BLE connection status
- Example configuration `ESP32_truma_cooler_example.yaml` (M5Stack Atom Lite)
- Note on optional parallel use as ESPHome Bluetooth Proxy (M5Stack Atom)

---


## [1.0.13] — 2026-04-20 — Truma Aventa Gen 2 air conditioning

### Added
- Support for the Truma Aventa Gen 2 air conditioner via the same LIN bus as the heater (CP Plus / iNet Box)
- New climate type `AIRCON` for a full HA climate entity (modes: Off / Cool / Heat / Heat+Cool / Fan only)
- New select types `AIRCON_MODE` and `AIRCON_VENT_MODE` for direct access to operating mode and fan speed
- Example configuration `ESP32-S3_truma_Aventa_example.yaml`

---


## [1.0.12] — 2026-04-19 — LIN protocol and thread-safety fixes

### Fixed
- LIN multi-PDU length decoding corrected (operator precedence `&` vs. `<<`):
  multi-frame messages > 255 bytes were previously dropped with length 0
- Length guard added before `reinterpret_cast` on incoming Truma frames
- `micros()` comparisons made wraparound-safe across the 71-minute rollover
  using unsigned subtraction
- Thread safety: timestamp fields (`device_registered_`, `init_requested_`,
  `init_received_`, `update_time_`) converted to `std::atomic<uint32_t>`
  (accessed from UART task and main loop)
- LIN logging: post-increment bug (`len = len++`) fixed, CRC byte was not logged
- Printf format specifier `%S` → `%s` corrected

## [1.0.11] — 2026-04-17 — ESPHome 2026.4.0 compatibility

### Documentation
- Hardware documentation extended by user request to make replication easier

### Fixed
- Corrected `cg.templatable()` calls in `__init__.py` for ESPHome 2026.4.x:
  enum types (`HeatingMode`, `TargetTemp`, `EnergyMix`, `ElectricPowerLevel`) now
  use `_dummy_ns` references instead of `cg.uint16`/`cg.uint8`
- Resolved entity key collisions between identically named Sensor and Number/Select
  entities that caused `aioesphomeapi` crashes under ESPHome 2026.4.0
  (`AttributeError: 'NumberInfo'/'SelectInfo' has no attribute 'accuracy_decimals'`)

### Changed

**Background:** In ESPHome, every entity receives a unique key (hash of its name).
If two entities on the same device share the same name — even if they are different
types (Sensor vs. Number or Select) — their keys collide. As of ESPHome 2026.4.0,
the order in which `aioesphomeapi` builds the entity list changed, causing these
collisions to trigger crashes for the first time.

The following sensor names have been renamed in all example YAMLs:

| Old name (sensor) | New name (sensor) | Conflict with |
|---|---|---|
| `Target Room Temperature` | `Target Room Temperature Status` | Number |
| `Target Water Temperature` | `Target Water Temperature Status` | Number |
| `Electric Power Level` | `Electric Power Level Status` | Number |
| `Energy Mix` | `Energy Mix Status` | Select |

> **Migration note for existing installations:** After flashing, the old sensor
> entities will appear as "unavailable" in Home Assistant. These must be manually
> deleted and the new entities (suffixed with "Status") re-added to dashboards
> and automations.

---

## [1.0.10] — 2026-04-11 — Further cleanup

### Changed
- Removed duplicate macro definitions of `DIAGNOSTIC_FRAME_MASTER` / `DIAGNOSTIC_FRAME_SLAVE` from two `.cpp` files and consolidated them as a single `constexpr` in `LinBusListener.h`
- Replaced magic number `1440` with named constant `MINUTES_PER_DAY`
- Implemented `dump_data()` in the heater module: logs target temperatures, heating mode, energy mix, power level, and operating status at DEBUG level; logs error codes at WARN level
- Corrected misleading comment on the consistency guard in `action_heater_energy_mix()`
- Fixed wrong label `"Truma Climate"` in `TrumaWaterClimate::dump_config()` to `"Truma Water Climate"`
- Removed commented-out preset code and dead options loop

---

## [1.0.9] — 2026-04-02 — Cleanup

### Changed
- Replaced magic number timeouts with named `constexpr` constants
- Added `const` to local variable `lin_identifier`

---

## [1.0.8] — 2026-03-30 — Code quality

### Fixed
- Corrected wrong field assignments in response frames for energy mix and electric power level

### Changed
- Cleaned up typos throughout the codebase
- Removed outdated and resolved comments
- Revised and unified code comments

### Documentation
- Added reference to @kamahat and their fork in all READMEs

---

## [1.0.7] — 2026-03-28 — Minor improvements

### Fixed
- Reduced log level for "LIN CRC error on SID" from WARN to VERBOSE — not a real error, just the Truma responding too slowly (suggested by @kamahat)

### Documentation
- Added `min_version: 2026.3.1` to all example YAMLs
- Added CONTRIBUTING files (DE/EN/FR)

---

## [1.0.6] — 2026-03-27 — Robustness

### Fixed

#### `components/truma_inetbox/LinBusListener_esp_idf.cpp`
- `uartEventTask_`: fixed a startup crash on dual-core ESP32 where the task could
  call `xQueueReceive()` with a NULL queue handle before `uart_driver_install()`
  on core 1 had completed
- Added a 5-second timeout to the queue-wait loop: if the UART driver never
  becomes available (e.g. UART setup failed), the task now logs a clear error
  and exits cleanly instead of looping silently forever

---

## [1.0.5] — 2026-03-27 — Improvements

### Changed

#### Example YAMLs (all four)
- `refresh` in `external_components` changed from `0s` to `24h` — ESPHome checks for updates once a day
- Added two commented-out alternatives: `refresh: always` (for development) and `refresh: 0s` (no automatic updates)

---

## [1.0.4] — 2026-03-23 — Bugfixes

### Fixed

#### `components/uart/__init__.py`
- `validate_raw_data()`: second `isinstance(value, str)` check (dead code, never reachable) corrected to `isinstance(value, bytes)`

#### `README.md` / `README.en.md`
- Stale filename references `ESP32-S3_truma_6DE_example.yaml` → `ESP32-S3_truma_6DE_Diesel_example.yaml` (file was renamed earlier)

---

## [1.0.3] — 2026-03-22 — OTA, cleanup

### Added

#### All WiFi-based example YAMLs
- Added `ota` block (`platform: esphome`, password placeholder) to all WiFi-based example configurations

#### `README.md` / `README.en.md`
- Added OTA section explaining over-the-air updates and the password placeholder

### Removed

- `WomoLinControllerEthernet.yaml` — removed (Ethernet-specific, not maintained here)
- `WomoLinControllerEthernetMqtt.yaml` — removed (Ethernet-specific, not maintained here)
- `examples/` directory — removed (superseded by root-level example YAMLs)

---

## [1.0.2] — 2026-03-19 — Example configurations and documentation

### Added

#### `ESP32_truma_4-6_Gas_example.yaml` / `ESP32-S3_truma_4-6_Gas_example.yaml` (new)
- Gas variant of the example configurations using `HEATER_GAS` and `HEATER_ENERGY_MIX_GAS`
- Diesel-„Entkokung"/Rückstandsverbrennung script, sensor and buttons omitted (gas-only operation)

### Changed

#### `ESP32_truma_6DE_example.yaml` → `ESP32_truma_6DE_Diesel_example.yaml`
#### `ESP32-S3_truma_6DE_example.yaml` → `ESP32-S3_truma_6DE_Diesel_example.yaml`
- Renamed to make the diesel variant explicit

#### `components/truma_inetbox/__init__.py` / `components/uart/__init__.py`
- Added `synchronous=True` to all `register_action()` calls
  (ESPHome 2026.3.0 requires this parameter; all `play()` methods are synchronous)

#### `README.md` / `README.en.md`
- Restructured example configuration section into 2-step selection (energy mix → hardware)
- Added Gas/Diesel variant overview table
- Added Truma Combi 4 compatibility note
- Added compatibility disclaimer: tested with Truma Combi 6DE (2018, Eberspächer burner);
  newer Truma diesel generations without Eberspächer not verified
- Removed upstream intro paragraph (originated from Fabian-Schmidt repo)
- Editorial cleanup

---

## [1.0.1] — 2026-03-14 — ESPHome 2026.6 Compatibility (deprecation follow-up)

### Changed

#### `components/truma_inetbox/__init__.py`
- `CORE.using_esp_idf` → `CORE.is_esp32 and not CORE.using_arduino`
  Deprecated since ESPHome 2026.1 (behavior change in 2026.6). The condition targets
  ESP-IDF-only builds where the `ARDUINO_SERIAL_EVENT_TASK_*` macros are not provided
  by the framework.

#### `components/uart/__init__.py`
- `CORE.using_esp_idf` → `not CORE.using_arduino`
  Same deprecation fix in the UART type selector (`_uart_declare_type`).

#### `components/truma_inetbox/LinBusListener_esp_idf.cpp`
- Added `#ifndef` fallback defines for `ARDUINO_SERIAL_EVENT_TASK_STACK_SIZE` (4096)
  and `ARDUINO_SERIAL_EVENT_TASK_RUNNING_CORE` (0) so the file compiles even without
  the build flags as a safety net.

#### `components/truma_inetbox/climate/TrumaWaterClimate.cpp`
#### `components/truma_inetbox/climate/TrumaRoomClimate.cpp`
- `traits.set_supports_current_temperature(true)`
  → `traits.add_feature_flags(climate::CLIMATE_SUPPORTS_CURRENT_TEMPERATURE)`
  `set_supports_current_temperature` is deprecated in ESPHome 2025+.

---

## [1.0.0] — 2026-03-02 — ESPHome 2025.8+ / 2026.3.x Compatibility — Details

### Changed — `components/uart/`

#### `uart_component.h`
- `virtual int available()` → `virtual size_t available()` to match ESPHome 2025.8+ signature
- Added default (no-op) implementations for new virtual methods introduced in ESPHome 2025.8:
  `set_rx_full_threshold()`, `set_rx_timeout()`, `load_settings(bool)`, `load_settings()`

#### `uart_component.cpp`
- Updated `check_read_timeout_()` to use `size_t` comparisons (no unnecessary `int` casts)

#### `uart_component_esp_idf.h` _(critical)_
- Preprocessor guard changed: `USE_ESP_IDF` → `USE_ESP32_FRAMEWORK_ESP_IDF`
- Removed `SemaphoreHandle_t lock_` member (mutex removed from upstream in 2025.8)
- `int available()` → `size_t available()`
- Added `get_hw_serial_number()` directly to `IDFUARTComponent` base class
- Added declarations for `load_settings()`, `set_rx_full_threshold()`, `set_rx_timeout()`
- `uart_event_queue_` kept **unconditionally** (not guarded by `USE_UART_WAKE_LOOP_ON_RX`),
  because the LIN-bus BREAK detection task requires it at all times

#### `uart_component_esp_idf.cpp` _(critical)_
- Preprocessor guard changed: `USE_ESP_IDF` → `USE_ESP32_FRAMEWORK_ESP_IDF`
- `UART_SCLK_APB` → `UART_SCLK_DEFAULT` (ESP-IDF 5.x API change)
- `portTICK_RATE_MS` → `pdMS_TO_TICKS(20)` (removed from ESP-IDF 5.x)
- All `lock_` mutex take/give calls removed (~12 call-sites)
- `static uint8_t next_uart_num` → `static uart_port_t next_uart_num = UART_NUM_0`
  (ESP-IDF 5.x: `uart_port_t` is a scoped enum, no implicit `uint8_t` conversion)
- Postfix `++` on `uart_port_t` replaced with explicit cast:
  `next_uart_num = (uart_port_t)(next_uart_num + 1)`
- `int available()` → `size_t available()`
- Added implementations for `load_settings()`, `set_rx_full_threshold()`, `set_rx_timeout()`

#### `truma_uart_component_esp_idf.h`
- Preprocessor guard changed: `USE_ESP_IDF` → `USE_ESP32_FRAMEWORK_ESP_IDF`
- Removed `get_hw_serial_number()` (now provided by `IDFUARTComponent` base class)
- Retains `get_uart_event_queue()` exposing `&uart_event_queue_`

#### `uart_component_esp32_arduino.h` / `.cpp`
- `int available()` → `size_t available()`
- `check_logger_conflict()`: `logger::global_logger->get_hw_serial()` guarded with
  `#if defined(USE_LOGGER) && !defined(USE_ESP32)` — ESPHome 2026.1 removed
  `get_hw_serial()` from `Logger` for ESP32 (Arduino on ESP32 now builds on IDF)

#### `uart_component_rp2040.h` / `.cpp`
- `int available()` → `size_t available()`

#### `uart_component_esp8266.h` / `.cpp`
- `ESP8266UartComponent::available()`: `int` → `size_t`

---

### Changed — `components/truma_inetbox/`

#### POSIX integer type replacements (all 30 affected files)
- `u_int8_t` → `uint8_t`
- `u_int16_t` → `uint16_t`
- `u_int32_t` → `uint32_t`

These POSIX-style types (`u_int*_t`) are defined implicitly by glibc / BSD libc headers
that the Arduino toolchain includes automatically. The ESP-IDF 5.x GCC toolchain does
**not** make them available, causing 294 compile errors across 30 files.

Affected files include:
`LinBusProtocol.h`, `LinBusProtocol.cpp`, `LinBusListener.h`, `LinBusListener.cpp`,
`TrumaiNetBoxApp.h`, `TrumaiNetBoxApp.cpp`, `TrumaiNetBoxAppHeater.h/cpp`,
`TrumaiNetBoxAppAirconManual.h/cpp`, `TrumaiNetBoxAppAirconAuto.h/cpp`,
`TrumaiNetBoxAppClock.h/cpp`, `TrumaiNetBoxAppTimer.h/cpp`,
`TrumaStructs.h`, `TrumaEnums.h`, `TrumaStatusFrameBuilder.h`,
`TrumaStausFrameResponseStorage.h`, `helpers.h`, `helpers.cpp`,
`automation.h`, `time/TrumaTime.h`, and sensor/number/select/climate sub-components.

#### `LinBusListener_esp_idf.cpp`
- `#define QUEUE_WAIT_BLOCKING (portTickType) portMAX_DELAY`
  → `(TickType_t) portMAX_DELAY`
  (`portTickType` was renamed to `TickType_t` in FreeRTOS 10 / ESP-IDF 5.x)
- `uart_intr_config(uart_num, &uart_intr)` → `uart_intr_config((uart_port_t) uart_num, &uart_intr)`
  (ESP-IDF 5.x: `uart_intr_config` requires `uart_port_t`, no implicit `uint8_t` conversion)

#### `LinBusListener_esp32_arduino.cpp`
- `#define QUEUE_WAIT_BLOCKING (portTickType) portMAX_DELAY`
  → `(TickType_t) portMAX_DELAY`
  (same FreeRTOS rename, also affects Arduino on ESP32 which builds on ESP-IDF 5.x)

---

### Added

- `test_compile.yaml` — minimal test configuration for ESP32 Arduino framework builds
- `test_compile_idf.yaml` — minimal test configuration for ESP32 ESP-IDF framework builds

---

### Notes

- ESPHome **2026.1.x does not exist** on PyPI — version numbering jumps from 2025.10.x
  directly to 2026.2.x.
- ESPHome 2026.1 deprecated `CORE.using_esp_idf` (warning only; behavior changes in 2026.6).
  ESP32 Arduino now officially builds on top of ESP-IDF, so IDF features are available
  in both frameworks. The `uart_component_esp32_arduino` component continues to work
  as a custom override for this project.
- Installing ESPHome 2026.2.x in a Python venv additionally requires the
  `fatfs-ng` package (`pip install fatfs-ng`) as a transitive PlatformIO dependency.
