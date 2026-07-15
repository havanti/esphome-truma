# Troubleshooting — Truma iNet Box (LIN bus)

[🇩🇪 Deutsch](TROUBLESHOOTING.md) | 🇬🇧 English

This guide covers the most common problems when commissioning the heater component (`truma_inetbox`). For the physical wiring see the [troubleshooting section in the hardware guide](hardware/README.en.md#troubleshooting); for the cooler see the [sniffing guide](SNIFFING.en.md).

---

## Golden rule: always start with a serial DEBUG log

Almost every diagnosis depends on a **serial** boot log:

```yaml
logger:
  level: DEBUG
```

Connect the ESP via USB and capture ~30 seconds from power-on:

```bash
esphome logs <config>.yaml
```

**Important:** Errors that happen in `setup()` (e.g. UART initialization) only show up **in the serial log**. Logs over the API (Home Assistant add-on, `esphome logs` over the network) start after Wi-Fi is up and will only show `Component ... marked FAILED` — without the reason.

---

## Symptom overview

| Symptom | Likely cause | Section |
|---|---|---|
| CP Plus never shows the iNet Box as "green" / does not connect | Checksum, missing slave slot, or wiring | [1](#1-cp-plus-does-not-connect-never-green) |
| Log full of `Cannot update Truma` warnings | UART/LIN bus dead | [2](#2-cannot-update-truma--uart-marked-failed) |
| `Component uart marked FAILED` at startup | Bug in versions < 1.0.23 | [2](#2-cannot-update-truma--uart-marked-failed) |
| Boot loop with `flash read err, 1000` | Warm-reset artifact, not a hardware defect | [3](#3-boot-loop-with-flash-read-err-1000) |
| Crash loop at startup (esp32dev config) | ESP32-PICO-D4: GPIO16/17 are flash pins | [4](#4-esp32-pico-d4-gpio1617-not-usable) |
| Home Assistant loses the connection / `aioesphomeapi` errors | Entity name collision | [5](#5-entity-name-collision-api-crash) |

---

## 1. CP Plus does not connect (never "green")

The three most common causes, check in this order:

### 1a. `lin_checksum` must be `VERSION_2`

`VERSION_2` (LIN 2.x enhanced) is the component default and set in all example YAMLs. With `VERSION_1` the CP Plus **discards every frame the ESP sends** — a connection will never be established. If you changed the value: change it back.

### 1b. CP Plus needs a free slave slot (re-pairing)

The CP Plus registers LIN slaves **only during its initialization**. If no working iNet Box was on the bus during the last pairing, no slot is reserved — the ESP then cannot register, no matter how correctly it transmits.

**Solution — clear the slots and re-pair:**

1. **Disconnect all devices from the CP Plus** (heater, iNet Box/ESP, …)
2. Run the CP Plus initialization → the slots are now empty
3. **Reconnect everything** (incl. the ESP with a correct `VERSION_2` configuration)
4. Run the initialization **again**

Afterwards all devices should be visible on the CP Plus — provided everything is wired correctly. Initialization only takes a few seconds and must not hang.

### 1c. Check the wiring

- A LIN transceiver (TJA1020 module, FST T151, WomoLIN v2, …) is mandatory — the ESP cannot speak LIN directly.
- The transceiver needs **12 V directly from the vehicle's electrical system** (not the ESP's 5 V).
- RJ12: LIN → **pin 3**, GND → **pin 5**. Details in the [hardware guide](hardware/README.en.md).
- UART pins: ESP32 (`esp32dev`) `tx: 17` / `rx: 16`, ESP32-**S3** `tx: 18` / `rx: 8`. Do **not** cross TX/RX between ESP and transceiver.

**Diagnosis via DEBUG log:** If **no LIN frames at all** arrive → wiring/pins/12 V. If frames arrive but no connection is established → checksum (1a) or slot (1b).

### 1d. If everything is correct and it still fails: CP Plus firmware

Older CP Plus firmware revisions do not support the iNet Box. If wiring, `VERSION_2` and re-pairing are verifiably correct and initialization still never assigns a slot, the control panel firmware may be the cause. Firmware updates are only available through Truma service partners.

---

## 2. `Cannot update Truma` / UART `marked FAILED`

Persistent `Cannot update Truma` warnings mean: the app logic is running, but the LIN bus delivers nothing — usually because the UART component failed at startup.

- **Versions before 1.0.23:** bug — the UART configuration struct was created uninitialized; depending on the build, `uart_param_config()` failed and the UART component was `marked FAILED` at startup. The behavior was build-dependent (identical code could work in one build and fail in the next). **Fix: update to ≥ 1.0.23.**
- The actual error reason (`uart_param_config failed: ...`) appears **only in the serial boot log** — see the [golden rule](#golden-rule-always-start-with-a-serial-debug-log).
- `setup() finished successfully!` in the log does **not** rule out a dead UART.

---

## 3. Boot loop with `flash read err, 1000`

Looks dramatic, but is usually **not a defective flash** and not a power problem: the message is an artifact of rapid warm resets (e.g. watchdog resets after a crash in `setup()`).

**Procedure:** Power-cycle the ESP and capture a **cold power-on** over serial — that shows the real error. Read the complete raw log, don't just grep for keywords. If `esphome upload`/esptool flashes and verifies without errors, the flash is healthy — then the error is in setup (e.g. a pin conflict, see below).

---

## 4. ESP32-PICO-D4: GPIO16/17 not usable

On the **ESP32-PICO-D4**, GPIO16/17 are internally wired to the embedded flash. The default UART pins of the `esp32dev` examples (`tx: 17` / `rx: 16`) cause a crash loop in `setup()` on this chip.

**Solution:** Use a board with a **WROOM or WROVER module** (GPIO16/17 free) — the examples are verified on those. Alternatively switch to an ESP32-S3 (`tx: 18` / `rx: 8`).

---

## 5. Entity name collision (API crash)

A `sensor:` and a `number:`/`select:` with an **identical name** produce the same hash key — `aioesphomeapi` (Home Assistant) crashes on connect.

**Solution:** Name sensor readbacks with the suffix `Status` (e.g. `Target Room Temperature Status`), as done in the example YAMLs since version 1.0.20.

---

## Checklist before opening an issue

If nothing helps: [report a bug](https://github.com/havanti/esphome-truma/issues/new/choose) — with this information it goes fastest:

- [ ] Component version (release tag, e.g. `v1.0.23`) and ESPHome version
- [ ] Board (exact module: WROOM / WROVER / PICO-D4 / S3)
- [ ] CP Plus model and software version (readable in the CP Plus service menu)
- [ ] The YAML you use (secrets removed) — ideally an unmodified example
- [ ] **Serial** DEBUG boot log from power-on (~30 s)
