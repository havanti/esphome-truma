# ESPHome & Truma CP Plus — smart heating and more 🚐

[🇩🇪 Deutsch](README.md) | 🇬🇧 English

## Acknowledgements

A heartfelt thank you goes to **[Fabian Schmidt](https://github.com/Fabian-Schmidt)**, whose outstanding work on the original [esphome-truma_inetbox](https://github.com/Fabian-Schmidt/esphome-truma_inetbox) repository made all of this possible in the first place. This project is a fork of his work, and without his dedication and expertise none of this would exist. Thank you, Fabian!

This project also builds on the incredible groundwork laid by the [WomoLIN project](https://github.com/muccc/WomoLIN) and [Daniel Fett's inetbox.py](https://github.com/danielfett/inetbox.py), as well as [mc0110's inetbox2mqtt](https://github.com/mc0110/inetbox2mqtt) — their protocol research, log files and documentation have been invaluable.

---

## What this fork adds

This fork extends the original component with several real-world features developed during daily use in a motorhome with a Truma Combi 6DE and an ESP32-S3 board. The full working configuration is provided in [`ESP32-S3_truma_6DE_Diesel_example.yaml`](ESP32-S3_truma_6DE_Diesel_example.yaml).

### TPMS — Tire Pressure Monitoring via Bluetooth Proxy

The ESP32 doubles as a Bluetooth Low Energy (BLE) receiver for aftermarket TPMS sensors, eliminating the need for a separate gateway. Four sensors are monitored simultaneously (FL, FR, RR, RL), each reporting:

- Tire pressure in bar
- Tire temperature in °C
- Sensor battery voltage in V

The integration uses `esp32_ble_tracker` with passive scanning and parses the manufacturer-specific advertisement payload directly in a C++ lambda. All twelve sensor entities appear automatically in Home Assistant as diagnostic sensors.

To adapt this for your own sensors, replace the four MAC addresses in the `on_ble_advertise` blocks with those of your TPMS sensors. The payload decoding logic (pressure offset, scaling) may need adjustment depending on your sensor brand.

> Note: BLE scanning and the Truma LIN bus operate in parallel on the same chip. On an ESP32-S3 with OctalSPI PSRAM the BLE stack can be offloaded to PSRAM, which significantly reduces the risk of memory conflicts. The provided PSRAM `sdkconfig_options` in `ESP32-S3_truma_6DE_Diesel_example.yaml` are already configured for this.

### Diesel "De-coking" / Residue Combustion

When a Truma Combi is operated on diesel for extended periods, carbon deposits can build up in the combustion chamber and burner nozzle. Recommended maintenance:

- Run a monthly de-coking cycle (automated by this configuration, see below).
- Use high-quality diesel fuel, or add a cetane-boosting additive to the tank — a higher cetane number leads to cleaner combustion and reduces deposit buildup.

The built-in `script_diesel_decoking` automates this procedure:

1. Switches the energy mix to Diesel
2. Sets the room heater to 30 °C / HIGH mode for 45 minutes
3. Shuts the heater off afterwards
4. (open doors and windows ;-) )

Two buttons are exposed in Home Assistant:

| Button | Function |
|---|---|
| Start Diesel De-coking | Starts the 45-minute de-coking cycle |
| Abort Diesel De-coking | Aborts the cycle and turns off the heater |

A template sensor (Diesel De-coking Remaining Time, unit: min) counts down the remaining time and is visible in the Home Assistant dashboard and the built-in web UI.

### Onboard RGB LED — visual status indicator (ESP32-S3)

The [Waveshare ESP32-S3-DEV-KIT-N8R8](https://www.waveshare.com/wiki/ESP32-S3-DEV-KIT-N8R8) and many other ESP32-S3 development boards include an onboard WS2812 RGB LED (GPIO38) that can be used as a visual status indicator for the LIN bus without any additional hardware.

The example configuration drives this LED with two signals:

| Signal | Meaning |
|---|---|
| Green blink (every 2 s, 500 ms) | CP Plus connected — LIN bus active |
| Blue flash (300 ms) | TX command sent to heater |
| Off | CP Plus not connected |

**Advantages over a Home Assistant-only indicator:**

- Immediate visual feedback directly on the device, without any app or dashboard
- Shows whether the ESP is communicating at all, before WiFi or HA is available
- Useful during initial setup and on-site troubleshooting

**Note on the 90-second timeout:** The `CP_PLUS_CONNECTED` sensor reports `false` only 90 seconds after the last received LIN packet. If the CP Plus is physically disconnected, the LED therefore stays green for up to 90 seconds — this is the intended behaviour of the LIN bus protocol, designed to tolerate brief connection interruptions.

The implementation uses two ESPHome globals (`led_color`, `led_ticks`) and a 100 ms interval as LED driver, so that other intervals (alive indication) and switch actions (TX commands) can control the LED simply by setting these variables.

### Fine-tuning, Monitoring & Stability

The example configuration includes a number of production-hardened settings that are not part of the basic example:

WiFi resilience — A 5-minute interval checks for lost connectivity and performs a soft reconnect (`wifi.disable` → delay → `wifi.enable`) without rebooting the ESP. `reboot_timeout` is set to `0s` to prevent unexpected reboots during heating cycles.

WiFi RF tuning — Output power is fixed at `17 dB` and power-save mode is set to `light` for a reliable connection in a metal vehicle body.

System diagnostics — The following sensors are always available in Home Assistant:

| Sensor | Description |
|---|---|
| TR ESP32 Temperature | Internal chip temperature |
| TR WiFi Signal dB | Raw RSSI in dBm (updated every 60 s) |
| TR WiFi Signal | RSSI mapped to 0–100 % for easy dashboarding |
| Uptime Sensor | Time since last boot |

Home Assistant time sync — The ESP clock is kept in sync via the Home Assistant time platform, which is required for the timer actions to work correctly.

Built-in web UI — A local web server runs on port 80 (ESPHome Web Server v3) with `include_internal: true`, so all entities including internal diagnostics are visible directly in the browser without needing Home Assistant.

Template switches — Ready-to-use on/off switches for the room heater, water heater, and the built-in timer are included, making automation and dashboard integration straightforward.

Restart button — A one-click ESP restart button is exposed in Home Assistant for remote maintenance.

---

## Example configurations

This repository provides four ready-to-use example configurations for the Truma Combi heater family.
All use the ESP-IDF framework and pull the component directly from this repository.
Requires ESPHome >= 2026.3.0.

**Supported models:**

| Model           | Output          | Fuel                 | Electric (230 V) |
| --------------- | --------------- | -------------------- | ---------------- |
| Combi 4         | ~4 kW           | Gas (propane/butane) | ❌               |
| Combi 6         | ~6 kW           | Gas (propane/butane) | ❌               |
| Combi 4E        | ~4 kW (+Hybrid) | Gas                  | ✅               |
| Combi 6E        | ~6 kW (+Hybrid) | Gas                  | ✅               |
| Combi Diesel 4  | ~4 kW           | Diesel               | ❌               |
| Combi Diesel 6  | ~6 kW           | Diesel               | ❌               |
| Combi Diesel 4E | ~4 kW (+Hybrid) | Diesel               | ✅               |
| Combi Diesel 6E | ~6 kW (+Hybrid) | Diesel               | ✅               |

> **Compatibility note:** Developed and tested with a Truma Combi 6DE (2018 model year, Eberspächer burner). Whether other models — in particular newer diesel generations with a burner developed by Truma themselves (without Eberspächer) — are also compatible has not been verified. Feedback on this is very welcome — please open a [GitHub Issue](https://github.com/havanti/esphome-truma/issues).

### Step 1: Choose the energy mix variant

Choose the variant that matches your vehicle. The **E variants** (4E / 6E / DE) additionally support electric operation — the relevant entities (`HEATER_ELECTRICITY`, `ELECTRIC_POWER_LEVEL`) are already included in both example configurations.

| Variant | ESP32 | ESP32-S3 |
|---|---|---|
| **Gas** | [`ESP32_truma_4-6_Gas_example.yaml`](ESP32_truma_4-6_Gas_example.yaml) | [`ESP32-S3_truma_4-6_Gas_example.yaml`](ESP32-S3_truma_4-6_Gas_example.yaml) |
| **Diesel** | [`ESP32_truma_6DE_Diesel_example.yaml`](ESP32_truma_6DE_Diesel_example.yaml) | [`ESP32-S3_truma_6DE_Diesel_example.yaml`](ESP32-S3_truma_6DE_Diesel_example.yaml) |

The variant determines which ESPHome entities are enabled:

| | Gas variant | Diesel variant |
|---|---|---|
| Binary sensor (fuel) | `HEATER_GAS` | `HEATER_DIESEL` |
| Energy mix select | `HEATER_ENERGY_MIX_GAS` | `HEATER_ENERGY_MIX_DIESEL` |
| Diesel "de-coking" / residue combustion (ESP32-S3 only) | not included | included |

> Note: Use only **one** energy mix select entity per configuration — either gas or diesel, not both at the same time.

### Step 2: Choose the hardware variant

| Feature | ESP32 | ESP32-S3 |
|---|---|---|
| Target chip | ESP32 (classic, rev ≥ 3) | ESP32-S3 |
| Board | `esp32dev` | `esp32-s3-devkitc-1` |
| PSRAM | not used | OctalSPI PSRAM enabled (N16R8, 8 MB) |
| BLE stack | in internal RAM | offloaded to PSRAM |
| LIN UART TX pin | GPIO17 | GPIO18 (avoids PSRAM pin conflict) |
| LIN UART RX pin | GPIO16 | GPIO8 (avoids PSRAM pin conflict) |
| Minimum chip revision | optional (`CONFIG_ESP32_REV_MIN`, commented out) | no restriction |
| Onboard RGB LED | not available | WS2812, GPIO38, LIN bus status indicator |
| Diesel "de-coking" / residue combustion | not included | included (diesel variant only) |
| Log level | `DEBUG` | `DEBUG` |

Use the ESP32 variant if you have a standard ESP32 (WROOM-32, DevKit etc.) without PSRAM.
Uncommenting `CONFIG_ESP32_REV_MIN: "3"` and `version: recommended` can reduce binary size on older toolchains.

Use the ESP32-S3 variant if you have an ESP32-S3 module with OctalSPI PSRAM (e.g. N16R8).
The PSRAM configuration (OCT mode, 80 MHz) is required for this module variant.
The UART pins have been moved away from GPIO16/17, which are reserved for PSRAM on S3 boards.

### Prerequisites

All configurations use `secrets.yaml` for WiFi credentials. Create a `secrets.yaml` in the
same directory with:

```yaml
wifi_WOMO_WLAN_ssid: "YourMobileSSID"
wifi_WOMO_password: "YourMobilePassword"
wifi_Home_ssid: "YourHomeSSID"
wifi_Home_password: "YourHomePassword"
api_encryption_key: ""
```

The `api` encryption key can be left empty for local use or filled with a 32-byte base64 key
generated by ESPHome.

### OTA (Over-the-Air Update)

All example configurations include an `ota` block that allows firmware updates directly over Wi-Fi — without physically connecting the ESP32:

```yaml
ota:
  platform: esphome
  password: "12345678901234567890123456789012"
```

**Important:** The password in the example files is a placeholder. Replace it with your own long password before deployment and store it safely. If you lose the password, you will need to flash the ESP32 via USB again.

### Minimal example

```yaml
esphome:
  name: "esphome-truma"

external_components:
  - source:
      type: git
      url: https://github.com/havanti/esphome-truma.git
    components: [truma_inetbox, uart]
    refresh: 0s

esp32:
  board: esp32dev
  framework:
    type: esp-idf
    version: recommended

uart:
  - id: lin_uart_bus
    tx_pin: 17
    rx_pin: 16
    baud_rate: 9600
    stop_bits: 2

truma_inetbox:
  uart_id: lin_uart_bus
  lin_checksum: VERSION_2

binary_sensor:
  - platform: truma_inetbox
    name: "CP Plus alive"
    type: CP_PLUS_CONNECTED

sensor:
  - platform: truma_inetbox
    name: "Current Room Temperature"
    type: CURRENT_ROOM_TEMPERATURE
  - platform: truma_inetbox
    name: "Current Water Temperature"
    type: CURRENT_WATER_TEMPERATURE
```

## ESPHome components

This project contains the following ESPHome components:

- `truma_inetbox` has the following settings:
  - `cs_pin` (optional) if you connect the pin of your lin driver chip.
  - `fault_pin` (optional) if you connect the pin of your lin driver chip.
  - `on_heater_message` (optional) [ESPHome Trigger](https://esphome.io/guides/automations.html) when a message from CP Plus is recieved.

Requires ESPHome 2026.3.0 or higher.

### Binary sensor

Binary sensors are read-only.

```yaml
binary_sensor:
  - platform: truma_inetbox
    name: "CP Plus alive"
    type: CP_PLUS_CONNECTED
```

The following `type` values are available:

- `CP_PLUS_CONNECTED`
- `HEATER_ROOM`
- `HEATER_WATER`
- `HEATER_GAS`
- `HEATER_DIESEL`
- `HEATER_MIX_1`
- `HEATER_MIX_2`
- `HEATER_ELECTRICITY`
- `HEATER_HAS_ERROR`
- `TIMER_ACTIVE`
- `TIMER_ROOM`
- `TIMER_WATER`

### Climate

Climate components support read and write.

```yaml
climate:
  - platform: truma_inetbox
    name: "Truma Room"
    type: ROOM
  - platform: truma_inetbox
    name: "Truma Water"
    type: WATER
```

The following `type` values are available:

- `ROOM`
- `WATER`

### Number

Number components support read and write.

```yaml
number:
  - platform: truma_inetbox
    name: "Target Room Temperature"
    type: TARGET_ROOM_TEMPERATURE
```

The following `type` values are available:

- `TARGET_ROOM_TEMPERATURE`
- `TARGET_WATER_TEMPERATURE`
- `ELECTRIC_POWER_LEVEL`
- `AIRCON_MANUAL_TEMPERATURE`

### Select

Select components support read and write.

```yaml
select:
  - platform: truma_inetbox
    name: "Fan Mode"
    type: HEATER_FAN_MODE_COMBI
```

The following `type` values are available:

- `HEATER_FAN_MODE_COMBI`
- `HEATER_FAN_MODE_VARIO_HEAT`
- `HEATER_ENERGY_MIX_GAS`
- `HEATER_ENERGY_MIX_DIESEL`

### Sensor

Sensors are read-only.

```yaml
sensor:
  - platform: truma_inetbox
    name: "Current Room Temperature"
    type: CURRENT_ROOM_TEMPERATURE
```

The following `type` values are available:

- `CURRENT_ROOM_TEMPERATURE`
- `CURRENT_WATER_TEMPERATURE`
- `TARGET_ROOM_TEMPERATURE`
- `TARGET_WATER_TEMPERATURE`
- `HEATING_MODE`
- `ELECTRIC_POWER_LEVEL`
- `ENERGY_MIX`
- `OPERATING_STATUS`
- `HEATER_ERROR_CODE`

### Actions

The following [ESP Home actions](https://esphome.io/guides/automations.html#actions) are available:

- `truma_inetbox.heater.set_target_room_temperature`
  - `temperature` - Temperature between 5C and 30C. Below 5C will disable the Heater.
  - `heating_mode` - Optional set heating mode: `"OFF"`, `ECO`, `HIGH`, `BOOST`.
- `truma_inetbox.heater.set_target_water_temperature`
  - `temperature` - Set water temp as number: `0`, `40`, `60`, `80`.
- `truma_inetbox.heater.set_target_water_temperature_enum`
  - `temperature` - Set water temp as text: `"OFF"`, `ECO`, `HIGH`, `BOOST`.
- `truma_inetbox.heater.set_electric_power_level`
  - `watt` - Set electricity level to `0`, `900`, `1800`.
- `truma_inetbox.heater.set_energy_mix`
  - `energy_mix` - Set energy mix to: `GAS`, `MIX`, `ELECTRICITY`.
  - `watt` - Optional: Set electricity level to `0`, `900`, `1800`
- `truma_inetbox.aircon.manual.set_target_temperature`
  - `temperature` - Temperature between 16C and 31C. Below 16C will disable the Aircon.
- `truma_inetbox.timer.disable` - Disable the timer configuration.
- `truma_inetbox.timer.activate` - Set a new timer configuration.
  - `start` - Start time.
  - `stop` - Stop time.
  - `room_temperature` - Temperature between 5C and 30C.
  - `heating_mode` - Optional: Set heating mode: `"OFF"`, `ECO`, `HIGH`, `BOOST`.
  - `water_temperature` - Optional: Set water temp as number: `0`, `40`, `60`, `80`.
  - `energy_mix` - Optional: Set energy mix to: `GAS`, `MIX`, `ELECTRICITY`.
  - `watt` - Optional: Set electricity level to `0`, `900`, `1800`.
- `truma_inetbox.clock.set` - Update CP Plus from ESP Home. You *must* have another [clock source](https://esphome.io/#time-components) configured like Home Assistant Time, GPS or DS1307 RTC.

## Feedback & Testing

If you give this component a try, your feedback is very welcome!

Please test it with your setup and let us know how it goes — whether everything works smoothly
or you run into any issues. Feel free to open an issue on
[GitHub](https://github.com/havanti/esphome-truma/issues) with your findings,
bug reports, or suggestions for improvement. Every report helps make this project better for everyone.

---

## Trademark Notice

TRUMA is a registered trademark owned by Truma Gerätetechnik GmbH & Co. KG, a Putzbrunn-based entity. This project is an independent, community-driven open-source effort and is neither affiliated with, endorsed by, nor supported by Truma Gerätetechnik GmbH & Co. KG. The use of the name "Truma" in this repository is solely for the purpose of technical identification and compatibility description.

## Disclaimer

Use of this project is entirely voluntary and at your own risk.

This software is provided "as is", without warranty of any kind, express or implied. The author(s) accept no liability whatsoever for any damage to persons, property, vehicles, heating appliances, or any other assets arising from the use, misuse, or inability to use this software or the configurations provided herein. This includes, but is not limited to, damage resulting from incorrect configuration, unexpected device behaviour, software bugs, or hardware failure.

Before using any automation that controls a gas or diesel heater, ensure you understand the operation of your specific device and comply with all applicable safety regulations. Always test new configurations under supervised conditions.
