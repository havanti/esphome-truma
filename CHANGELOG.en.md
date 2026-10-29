# Changelog

[🇩🇪 Deutsch](CHANGELOG.md) | 🇬🇧 English

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

---

## Compatibility Status

### Summary

New in 1.0.50: prepared for ESPHome 2026.11 (PR #42 by Marco). Since 1.0.49 the timer can be set from the
dashboard in the heater examples, according to feedback in issue #39 it works. Since 1.0.48 HA shows the current temperature as unknown instead of 0 °C without data from the
heater, the Combi 6 spike after a wake-up is filtered out (PR #41 by Marco, not confirmed in a vehicle yet).
Since 1.0.47 the current temperature shows up in HA with tenths, the water also over MQTT (PR #40 by Marco).
Since 1.0.46 the climate automatic of the CP Plus can be set from HA again, according to feedback in
issue #32 it works. Since 1.0.42 the CP Plus C3 works as well (issue #29, PR #30 by Marco). Marco
also confirmed the fan mode select on the Combi 6 Gas. `HEATER_HAS_ERROR` now also reports warnings.
The shore power sensor of the Combi D from 1.0.39 is not confirmed yet. AUTO display and light of the
Aventa are confirmed (issue #28).

Tested against:
- ESPHome **2026.10.1** — ESP-IDF ✅

---


## [1.0.50] — 2026-10-29 — Preparing for ESPHome 2026.11

### Changed
- `truma_inetbox`, `truma_cooler`: Prepared for ESPHome 2026.11, a header is now included directly
  (PR #42 by Marco).

## [1.0.49] — 2026-10-29 — Timer in the dashboard

### Added
- Heater examples: start, stop, room temperature, heating mode and hot water of the timer can be set from the
  dashboard, while the timer is active changes are sent to the CP Plus right away (idea by Gerald, issue #39).
  According to feedback in issue #39 it works.

## [1.0.48] — 2026-10-28 — Current temperature without spikes

### Changed
- `truma_inetbox`: when the CP Plus has no data from the heater (0.0 °C room and water), HA shows the
  current temperatures as unknown instead of 0 °C (PR #41 by Marco).
- `truma_inetbox`: Combi 6 H5.00.00: the single 31.1/50 °C spike after the bus wakes up is filtered out, the
  last current values stay (issue #37, PR #41 by Marco). Not confirmed in a vehicle yet.

## [1.0.47] — 2026-10-27 — Current temperature in HA

### Fixed
- `truma_inetbox`: `ROOM` shows the current temperature with tenths, `WATER` also shows up over MQTT
  (issue #38, PR #40 by Marco). Tested by Marco.

### Changed
- `truma_inetbox`: `AIRCON` and `AIRCON_AUTO` as well. Not confirmed on an Aventa yet.

## [1.0.46] — 2026-10-24 — Aventa: set AUTO from HA (experimental)

### Added
- `truma_inetbox`: Climate entity `AIRCON_AUTO` and number `AIRCON_AUTO_TEMPERATURE` switch the climate
  automatic of the CP Plus on and off again and set the target (18–25 °C). The format is taken from
  captures of an original iNet Box (issue #32). According to feedback in the issue it works.

### Documentation
- README and Aventa example cover AUTO from HA, the entities are commented out in the example.
  `VENT_MODE` shows levels 1–10 on another Combi as well (issue #32).

## [1.0.45] — 2026-10-15 — Cleanup

### Changed
- `truma_inetbox`: The spellings `1.8kW`, `1,8 kW` for `watt` are accepted now.
- `truma_inetbox`: `CP_PLUS_CONNECTED` stays off after the CP Plus disappears until requests arrive
  again. Before, the sensor briefly showed "connected" about every 71 minutes.
- `truma_inetbox`: No second warning when the CP Plus fetches an update but none is pending.
- Code cleanup: named constants instead of numbers, the AUTO send path unused since 1.0.41 and other
  dead parts removed, comments corrected. No change in behaviour.

## [1.0.44] — 2026-10-15 — Follow-up commands, robustness

### Fixed
- `truma_inetbox`: Two commands shortly after each other (e.g. set a temperature, then switch on) no
  longer overwrite the first one with the switch-on default. Room, water and fan mode select confirmed
  on the bench, the Aventa takes the same path.
- `truma_inetbox`: The water number shows 80 instead of 200 on boost. Confirmed on the bench.

### Changed
- `truma_inetbox`: The fan modes of the room climate now also work on a Vario Heat (Night/Auto).
  Not confirmed yet.
- `truma_inetbox`: A custom, shorter `options:` list on a `select` is rejected during config
  validation, before the ESP kept restarting with it.
- `truma_inetbox`: `observer_mode` keeps the LIN transceiver awake via `cs_pin`, otherwise the ESP hears nothing.
- `truma_inetbox`: Timer times up to 23:59; a power level without energy mix is completed as for the heater.
- `truma_inetbox`: The Aventa number shows 15 instead of 0 when off.
- `truma_inetbox`: The time platform `time: platform: truma_inetbox` now sets the ESP clock correctly.

### Removed
- `uart`: The `debug:` option, it disturbed the LIN timing.

### Documentation
- README/examples: Aventa example without the removed AUTO control, actions with `DIESEL`/`NIGHT`/`AUTO`,
  outdated details (minimum version, sensor names, links) cleaned up.

## [1.0.43] — 2026-10-14 — Heating with a target, clock sync

### Fixed
- `truma_inetbox`: Heating with a temperature in one call (e.g. `climate.set_temperature` with
  `hvac_mode: heat`) now sets the requested temperature instead of 5 °C (water 40 °C). Confirmed on the bench.
- `truma_inetbox`: The clock is set again if HA was not connected at the first attempt. Confirmed on
  the bench.

## [1.0.42] — 2026-10-14 — CP Plus C3

### Fixed
- `truma_inetbox`: CP Plus C3: init and commands from HA also go through while the panel sleeps, the
  clock is not set there. Confirmed by Marco (issue #29), thanks to
  [Marco (@magior)](https://github.com/magior) for PR #30.

### Changed
- `truma_inetbox`: `HEATER_HAS_ERROR` also turns on for warnings, e.g. W255 (briefly when the CP Plus
  starts).
- `truma_inetbox`: The fan mode select sends Eco/High to a Combi 6 Gas instead of the Vario Heat modes.
  Confirmed by Marco (PR #30).

### Documentation
- README: CP Plus C3, `HEATER_HAS_ERROR` and thanks to Marco.

## [1.0.41] — 2026-10-09 — Aventa: AUTO from HA removed again

### Removed
- `truma_inetbox`: `AIRCON_AUTO` and `AIRCON_AUTO_TEMPERATURE` from 1.0.40, the CP Plus rejects the
  command (issue #28). Please delete them from your YAML.

### Documentation
- README and Aventa example updated.

## [1.0.40] — 2026-10-09 — Aventa: set AUTO from HA (experimental)

### Added
- `truma_inetbox`: climate entity `AIRCON_AUTO` and number `AIRCON_AUTO_TEMPERATURE` switch AUTO on
  the CP Plus on and off and set the target. Rejected by the CP Plus (issue #28), removed in 1.0.41.

### Documentation
- README and Aventa example extended with AUTO from HA, the entities are commented out in the example.

## [1.0.39] — 2026-10-07 — Combi D: shore power at the heater (experimental)

### Added
- `truma_inetbox`: binary sensor `HEATER_MAINS_POWER` (Combi D only) for 230 V shore power at the
  heater. Experimental, not confirmed yet.

### Changed
- `truma_inetbox`: `AIRCON_AUTO_TARGET_TEMPERATURE` shows "unknown" instead of 0 °C while AUTO is
  off. Confirmed on an Aventa Compact Plus 2nd Gen (issue #28).

### Documentation
- README: `HEATER_MAINS_POWER` and capturing bus traffic described.

## [1.0.38] — 2026-10-03 — Aventa: light and AUTO from the CP Plus

### Added
- `truma_inetbox`: Number `AIRCON_LIGHT` for the Aventa light (0 = off, 1–5). According to feedback
  this works on a first generation Aventa (issue #28).
- `truma_inetbox`: Binary sensor `AIRCON_AUTO_ACTIVE` and sensor `AIRCON_AUTO_TARGET_TEMPERATURE`
  show AUTO on the CP Plus. Confirmed on an Aventa Compact Plus 2nd Gen (issue #28).

### Fixed
- `truma_inetbox`: Aventa commands from HA leave the light unchanged, before it was presumably
  switched off. Confirmed according to feedback on a first generation Aventa.

### Documentation
- README: Aventa section and Aventa example extended with light and AUTO.

## [1.0.37] — 2026-09-30 — Code cleanup

### Changed
- `truma_inetbox`, `uart`: Code cleanup, no change in behavior. Runs since 1.0.38 on a CP Plus with
  an Aventa (issue #28).

## [1.0.36] — 2026-09-30 — Web server optional

### Changed
- Example YAMLs: The web server is commented out and optional. Without login anyone on the same
  Wi-Fi could use it, the example now contains a login from `secrets.yaml`.

### Documentation
- README: new section "Web server (optional)".

## [1.0.35] — 2026-09-29 — Water heater on Aventa commands

### Fixed
- `truma_inetbox`: Aventa commands from HA no longer switch off the water heater (issue #16, #28).
  Confirmed on an Aventa Compact Plus 2nd Gen.

### Documentation
- README: note that AUTO selected on the CP Plus is not evaluated.

## [1.0.34] — 2026-09-29 — Cooler fixes, LIN communication hardening

### Changed
- `truma_cooler`: Registering for status notifications goes through ESPHome instead of own code.
- `truma_inetbox`: Answers to the CP Plus are protected against concurrent access. Before, a
  corrupted answer or a crash was rarely possible.
- `truma_inetbox`: LIN reception calculates its timeout correctly also when the counter rolls over
  (about every 71 minutes).
- `truma_inetbox`, `uart`: UART setup failures now reliably show up in the log.

### Fixed
- `truma_cooler`: Sending "cool" again to a running box no longer switches off the turbo on a C44.
- `truma_cooler`: The turbo switch stays off when used while the box is off.
- `truma_cooler`: A C44 running turbo is shown as turbo, before compressor and turbo showed as off.
- `truma_cooler`: Changes after a command reach Home Assistant after about one second instead of up
  to a minute.

## [1.0.33] — 2026-09-28 — Switching on the Aventa

### Fixed
- `truma_inetbox`: The Aventa can now be switched on from HA also with CP Plus C.04.05.02, which
  rejected the command before (issue #28). Confirmed on an Aventa Compact Plus 2nd Gen.

### Documentation
- README: note that `VENT_MODE` has no values on a Combi D6 E.

## [1.0.32] — 2026-09-24 — Fan level

### Added
- `truma_inetbox`: Sensor `VENT_MODE` shows the fan level (0 off, 1–10, 11 heater fan Eco, 13 heater
  fan High). Display only, checked on a Combi 4 (issue #25).

### Documentation
- README: `HEATING_DEMAND` stays off in water-heater-only mode.
- README: `PID22_BYTE0` is the supply voltage, confirmed by a second measurement.

## [1.0.31] — 2026-09-23 — Example configurations for ESPHome 2026.9.0

### Changed
- Example YAMLs: The API key comes from `!secret api_encryption_key`, ESPHome 2026.9.0 rejects the
  empty key.
- Example YAMLs: OTA uses the API encryption instead of a password. Switching over: see README,
  section OTA.
- Example YAMLs: Shorter BLE scan window against Wi-Fi drops, `channel_colors` instead of
  `rgb_order`.

### Documentation
- README: Prerequisites and OTA adapted to the new examples.

## [1.0.30] — 2026-09-23 — Heat demand (experimental)

### Added
- `truma_inetbox`: `HEATING_DEMAND` (heat request, not burner status) and `PID22_BYTE0`/`PID22_BYTE1`.
  Experimental, Combi 4 only (issue #25).

## [1.0.29] — 2026-09-21 — Unused function warnings

### Fixed
- `truma_inetbox`: No more `-Wunused-function` warnings during the build.

## [1.0.28] — 2026-09-21 — Format warnings in the UART component

### Fixed
- `uart`: No more `-Wformat` warnings when building for the ESP32-S3. The logged values were correct
  before as well.

## [1.0.27] — 2026-09-19 — Actions in triggers with arguments

### Fixed
- `truma_inetbox`, `uart`: With ESPHome 2026.9 the actions also work in triggers with arguments,
  e.g. in `api:` actions or `on_value`. Before, the build failed there.

## [1.0.26] — 2026-09-15 — Log PID fixed, `OPERATING_STATUS` corrected

### Fixed
- `truma_inetbox`: Wrong assignment in the VERBOSE log, only the log output was affected.

### Documentation
- README: `OPERATING_STATUS` from 5 upwards does not show a running burner, the values depend on the
  model (issue #25).

## [1.0.25] — 2026-09-07 — Example YAMLs: `id` for Operating Status

### Documentation
- Heater examples: The sensor "Operating Status" has `id: operating_status` for lambdas.
- README: Values of `OPERATING_STATUS` explained, corrected in 1.0.26.

## [1.0.24] — 2026-07-17 — Fixes from code audit

### Fixed
- `truma_inetbox`: "CP Plus connected" briefly reported `false` by mistake about every 71 minutes.

## [1.0.23] — 2026-07-09 — UART startup failure fixed

### Fixed
- `uart`: Depending on the build, the UART component could end up `FAILED` at startup, then there
  was no LIN traffic ("Cannot update Truma"). The cause only showed in the serial boot log (issue
  #21).
- `truma_inetbox`: Build error with `logger: level: VERBOSE` fixed (issue #22).

### Changed
- The log line `Component version:` shows the correct version again, it was stuck at 1.0.20.

## [1.0.22] — 2026-07-03 — Truma Cooler C69: master power switch + compressor confirmed

### Added
- `truma_cooler` C69: Switch `power` turns the whole box on and off (issue #18).

### Changed
- `truma_cooler` C69: The zone climates only cool and set the target temperature, on/off goes
  through `power`. Existing C69 configurations need the `power` switch, see the example.
- `truma_cooler` C69: Compressor status confirmed on real hardware (issue #18).

## [1.0.21] — 2026-07-03 — Truma Cooler: C69 (dual zone) via `model:` selector

### Added
- `truma_cooler`: Selector `model:` (`c44` default, `c69`). C44 configurations without `model:` keep
  working unchanged.
- `truma_cooler` C69: two zones with their own target and actual temperature, without turbo. Example
  `ESP32_truma_cooler_C69_example.yaml`.

## [1.0.20] — 2026-05-12 — Truma Cooler: hardening and state after restart

### Added
- `truma_cooler`: Mode and target temperature survive a restart, turbo and device status start as
  "off" instead of "unknown".

### Changed
- `truma_cooler`: BLE events only change entities from the main loop.

### Fixed
- `truma_cooler`: Quickly switching on and off no longer sends a needless turbo-off command.

## [1.0.19] — 2026-05-12 — RP2040 support removed

### Removed
- Support for the Raspberry Pi Pico (RP2040), the repo is ESP-IDF only.

## [1.0.18] — 2026-04-29 — Thread safety in heater/aircon/clock

### Changed
- `truma_inetbox`: Protected against concurrent access.

## [1.0.17] — 2026-04-29 — Truma Cooler: robustness fixes

### Changed
- `truma_cooler`: Code cleanup, protected against concurrent access.

### Fixed
- `truma_cooler`: Turbo commands are ignored while the box is off.
- `truma_cooler`: After connecting, the climate shows "off" instead of "unknown".
- `truma_cooler`: A disconnect no longer leaves a pending turbo reset behind.

## [1.0.16] — 2026-04-26 — Version display in web interface

### Added
- Text sensor shows the component version in Home Assistant and the web interface, all heater
  examples include it. The version is also logged at startup.

## [1.0.15] — 2026-04-23 — Safety and robustness fixes

### Fixed
- `truma_inetbox`: Incomplete LIN data is dropped.
- `truma_inetbox`: The operating status "ON 255" was shown as "ON 25".
- `truma_inetbox`: Without a time server, the clock stayed in update state forever.
- `truma_cooler`: The target temperature is applied also when the mode changes at the same time.
- `truma_inetbox`, `truma_cooler`: Protected against concurrent access.

## [1.0.14] — 2026-04-21 — Truma Cooler C(XX) integration

### Added
- New component `truma_cooler` for the Truma Cooler C(XX) via BLE: climate (−22 to +10 °C), inside
  and outside temperature, compressor, turbo, device and connection status.
- Example `ESP32_truma_cooler_example.yaml` (M5Stack Atom Lite), can be used as Bluetooth proxy at
  the same time.

## [1.0.13] — 2026-04-20 — Truma Aventa Gen 2 air conditioning

### Added
- Truma Aventa Gen 2 over the same LIN bus as the heater: climate `AIRCON`, selects `AIRCON_MODE`
  and `AIRCON_VENT_MODE`, number `AIRCON_MANUAL_TEMPERATURE`. Example
  `ESP32-S3_truma_Aventa_example.yaml`.

## [1.0.12] — 2026-04-19 — LIN fixes

### Fixed
- `truma_inetbox`: Long LIN messages were dropped, short ones not checked.
- `truma_inetbox`: Time comparisons are correct also after the counter rolls over (about every 71
  minutes).
- `truma_inetbox`: The checksum was missing in the log.

## [1.0.11] — 2026-04-17 — ESPHome 2026.4.0 compatibility

### Fixed
- `truma_inetbox`: Build error with ESPHome 2026.4 fixed.
- Example YAMLs: Sensors with the same name as a number or select crashed the HA integration with
  ESPHome 2026.4.0. `Target Room Temperature`, `Target Water Temperature`, `Electric Power Level`
  and `Energy Mix` now carry the suffix "Status" as sensors. After flashing, delete the old sensors
  in HA and switch dashboards and automations to the new ones.

### Documentation
- Hardware documentation extended to make rebuilding easier.

## [1.0.10] — 2026-04-11 — Further cleanup

### Changed
- `truma_inetbox`: Code cleanup. The heater module logs its state at DEBUG, error codes at WARN.

## [1.0.9] — 2026-04-02 — Cleanup

### Changed
- Code cleanup, no change in behavior.

## [1.0.8] — 2026-03-30 — Code quality

### Fixed
- `truma_inetbox`: Energy mix and electric power level were set wrongly in the answer to the CP
  Plus.

### Changed
- Typos and comments cleaned up.

### Documentation
- README mentions the fork of @kamahat.

## [1.0.7] — 2026-03-28 — Minor improvements

### Fixed
- "LIN CRC error on SID" only appears in the VERBOSE log, it is not a real error (suggested by
  @kamahat).

### Documentation
- `min_version: 2026.3.1` in all examples, CONTRIBUTING files added.

## [1.0.6] — 2026-03-27 — Robustness

### Fixed
- `truma_inetbox`: Crash at startup on dual-core ESP32 fixed. If the UART driver does not come up, a
  clear message is logged after 5 s.

## [1.0.5] — 2026-03-27 — Improvements

### Changed
- Example YAMLs: `refresh: 24h` instead of `0s`, alternatives as comments.

## [1.0.4] — 2026-03-23 — Bugfixes

### Fixed
- `uart`: Raw data check for `uart.write` corrected.
- README: Outdated name of the diesel example file corrected.

## [1.0.3] — 2026-03-22 — OTA, cleanup

### Added
- OTA in all Wi-Fi examples, plus a section in the README.

### Removed
- WomoLin Ethernet examples and the `examples/` directory.

## [1.0.2] — 2026-03-19 — Example configurations and documentation

### Added
- Gas examples for ESP32 and ESP32-S3.

### Changed
- The diesel examples are now named `*_6DE_Diesel_example.yaml`.
- Actions adapted to ESPHome 2026.3.0.

### Documentation
- README: Choosing the example by energy source and hardware, notes on the Combi 4 and on diesel
  models without Eberspächer burner.

## [1.0.1] — 2026-03-14 — ESPHome 2026.6 compatibility

### Changed
- Deprecated ESPHome APIs replaced so the components also build with ESPHome 2026.6.

## [1.0.0] — 2026-03-02 — ESPHome 2025.8+ / 2026.3.x compatibility

### Added
- Test configurations `test_compile.yaml` and `test_compile_idf.yaml`.

### Changed
- `uart`: Adapted to ESPHome 2025.8+ and ESP-IDF 5.x. The own UART component is still needed because
  LIN detection requires the event queue.
- `truma_inetbox`: Builds with ESP-IDF 5.x.
