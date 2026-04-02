# Changelog

🇩🇪 Deutsch | [🇬🇧 English](CHANGELOG.en.md) | [🇫🇷 Français](CHANGELOG.fr.md)

Alle wesentlichen Änderungen an diesem Projekt werden in dieser Datei dokumentiert.

Das Format basiert auf [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

---

## Kompatibilitätsstatus

### Zusammenfassung

Dieses Release stellt die Kompatibilität mit ESPHome 2025.8 bis 2026.3.x wieder her.
Hauptursache war die Entfernung von `get_uart_event_queue()` aus der upstream
`IDFUARTComponent` in ESPHome 2025.8, wodurch die LIN-Bus-BREAK-Erkennung bei
ESP-IDF-Builds nicht mehr funktionierte. Zusätzliche Breaking Changes in ESP-IDF 5.x
(ESP32-Toolchain) und ESPHome 2026.x API-Änderungen wurden ebenfalls behoben.

Getestet mit:
- ESPHome **2026.3.2** — ESP-IDF ✅
- ESPHome **2026.3.1** — ESP-IDF ✅
- ESPHome **2026.3.0** — ESP-IDF ✅
- ESPHome **2026.2.2** — ESP-IDF ✅
- ESPHome **2025.9.3** — ESP-IDF ✅

---

## [1.0.8] — 2026-03-30 — Codequalität

### Behoben
- Falsche Feldzuweisungen in den Antwort-Frames für Energiemix und elektrische Leistungsstufe behoben

### Geändert
- Tippfehler im gesamten Codebase bereinigt
- Veraltete und erledigte Kommentare entfernt
- Codekommentare überarbeitet und vereinheitlicht

### Dokumentation
- READMEs um Hinweis auf @kamahat und dessen Fork ergänzt

---

## [1.0.7] — 2026-03-28 — Kleinere Verbesserungen

### Behoben
- Log-Level für „LIN CRC error on SID" von WARN auf VERBOSE gesenkt — kein echter Fehler, nur eine zu langsame Truma-Antwort (vorgeschlagen von @kamahat)

### Dokumentation
- `min_version: 2026.3.1` in allen Beispiel-YAMLs ergänzt
- CONTRIBUTING-Dateien (DE/EN/FR) hinzugefügt

---

## [1.0.6] — 2026-03-27 — Robustheit

### Behoben

#### `components/truma_inetbox/LinBusListener_esp_idf.cpp`
- `uartEventTask_`: Absturz beim Start auf Dual-Core-ESP32 behoben, bei dem der Task
  `xQueueReceive()` mit einem NULL-Queue-Handle aufrufen konnte, bevor `uart_driver_install()`
  auf Core 1 abgeschlossen war
- Timeout von 5 Sekunden zur Queue-Warteloop hinzugefügt: falls der UART-Treiber nie
  verfügbar wird (z. B. bei fehlgeschlagenem UART-Setup), loggt der Task jetzt eine
  klare Fehlermeldung und beendet sich sauber, anstatt still weiterzulaufen

---

## [1.0.5] — 2026-03-27 — Verbesserungen

### Geändert

#### Beispiel-YAMLs (alle vier)
- `refresh` in `external_components` von `0s` auf `24h` geändert — ESPHome prüft einmal täglich auf Updates
- Zwei auskommentierte Alternativen ergänzt: `refresh: always` (für Entwicklung) und `refresh: 0s` (kein automatisches Update)

---

## [1.0.4] — 2026-03-23 — Fehlerbehebungen

### Behoben

#### `components/uart/__init__.py`
- `validate_raw_data()`: zweiter `isinstance(value, str)`-Check (toter Code, nie erreichbar) korrigiert zu `isinstance(value, bytes)`

#### `README.md` / `README.en.md`
- Veralteter Dateiname `ESP32-S3_truma_6DE_example.yaml` → `ESP32-S3_truma_6DE_Diesel_example.yaml` (Datei wurde zuvor umbenannt)

---

## [1.0.3] — 2026-03-22 — OTA, Aufräumen

### Hinzugefügt

#### Alle WiFi-basierten Beispiel-YAMLs
- `ota`-Block (`platform: esphome`, Passwort-Platzhalter) zu allen WiFi-basierten Beispielkonfigurationen hinzugefügt

#### `README.md` / `README.en.md`
- OTA-Abschnitt ergänzt: Erklärung von Over-the-Air-Updates und Hinweis zum Passwort-Platzhalter

### Entfernt

- `WomoLinControllerEthernet.yaml` — entfernt (Ethernet-spezifisch, wird hier nicht gepflegt)
- `WomoLinControllerEthernetMqtt.yaml` — entfernt (Ethernet-spezifisch, wird hier nicht gepflegt)
- Verzeichnis `examples/` — entfernt (durch Root-Level-Beispiel-YAMLs ersetzt)

---

## [1.0.2] — 2026-03-19 — Beispielkonfigurationen und Dokumentation

### Hinzugefügt

#### `ESP32_truma_4-6_Gas_example.yaml` / `ESP32-S3_truma_4-6_Gas_example.yaml` (neu)
- Gas-Variante der Beispielkonfigurationen mit `HEATER_GAS` und `HEATER_ENERGY_MIX_GAS`
- Diesel-„Entkokung"/Rückstandsverbrennung (Script, Sensor, Buttons) nicht enthalten (nur Gasbetrieb)

### Geändert

#### `ESP32_truma_6DE_example.yaml` → `ESP32_truma_6DE_Diesel_example.yaml`
#### `ESP32-S3_truma_6DE_example.yaml` → `ESP32-S3_truma_6DE_Diesel_example.yaml`
- Umbenannt, um die Diesel-Variante explizit kenntlich zu machen

#### `components/truma_inetbox/__init__.py` / `components/uart/__init__.py`
- `synchronous=True` zu allen `register_action()`-Aufrufen hinzugefügt
  (ESPHome 2026.3.0 erfordert diesen Parameter; alle `play()`-Methoden sind synchron)

#### `README.md` / `README.en.md`
- Beispielkonfigurations-Abschnitt in 2-Schritt-Auswahl umstrukturiert (Energiemix → Hardware)
- Übersichtstabelle Gas-/Diesel-Variante ergänzt
- Kompatibilitätshinweis für Truma Combi 4 ergänzt
- Kompatibilitätsvorbehalt ergänzt: getestet mit Truma Combi 6DE (Baujahr 2018, Eberspächer-Brenner);
  neuere Truma-Diesel-Generationen ohne Eberspächer nicht verifiziert
- Einleitungsabsatz aus dem Upstream-Repo (Fabian-Schmidt) entfernt
- Redaktionelle Überarbeitung

---

## [1.0.1] — 2026-03-14 — ESPHome 2026.6 Kompatibilität (Deprecation-Nachfolge)

### Geändert

#### `components/truma_inetbox/__init__.py`
- `CORE.using_esp_idf` → `CORE.is_esp32 and not CORE.using_arduino`
  Seit ESPHome 2026.1 als veraltet markiert (Verhaltensänderung in 2026.6). Die Bedingung
  zielt auf ESP-IDF-only-Builds ab, in denen die `ARDUINO_SERIAL_EVENT_TASK_*`-Makros
  nicht vom Framework bereitgestellt werden.

#### `components/uart/__init__.py`
- `CORE.using_esp_idf` → `not CORE.using_arduino`
  Gleiche Deprecation-Korrektur im UART-Typ-Selektor (`_uart_declare_type`).

#### `components/truma_inetbox/LinBusListener_esp_idf.cpp`
- `#ifndef`-Fallback-Defines für `ARDUINO_SERIAL_EVENT_TASK_STACK_SIZE` (4096)
  und `ARDUINO_SERIAL_EVENT_TASK_RUNNING_CORE` (0) ergänzt, damit die Datei auch
  ohne Build-Flags kompiliert (Sicherheitsnetz).

#### `components/truma_inetbox/climate/TrumaWaterClimate.cpp`
#### `components/truma_inetbox/climate/TrumaRoomClimate.cpp`
- `traits.set_supports_current_temperature(true)`
  → `traits.add_feature_flags(climate::CLIMATE_SUPPORTS_CURRENT_TEMPERATURE)`
  `set_supports_current_temperature` ist ab ESPHome 2025+ veraltet.

---

## [1.0.0] — 2026-03-02 — ESPHome 2025.8+ / 2026.3.x Kompatibilität — Details

### Geändert — `components/uart/`

#### `uart_component.h`
- `virtual int available()` → `virtual size_t available()` entsprechend der ESPHome 2025.8+ Signatur
- Standard-(No-op-)Implementierungen für neue virtuelle Methoden aus ESPHome 2025.8 ergänzt:
  `set_rx_full_threshold()`, `set_rx_timeout()`, `load_settings(bool)`, `load_settings()`

#### `uart_component.cpp`
- `check_read_timeout_()` verwendet nun `size_t`-Vergleiche (keine unnötigen `int`-Casts)

#### `uart_component_esp_idf.h` _(kritisch)_
- Präprozessor-Guard geändert: `USE_ESP_IDF` → `USE_ESP32_FRAMEWORK_ESP_IDF`
- `SemaphoreHandle_t lock_`-Member entfernt (Mutex in Upstream 2025.8 entfernt)
- `int available()` → `size_t available()`
- `get_hw_serial_number()` direkt in `IDFUARTComponent`-Basisklasse ergänzt
- Deklarationen für `load_settings()`, `set_rx_full_threshold()`, `set_rx_timeout()` ergänzt
- `uart_event_queue_` **bedingungslos** behalten (nicht durch `USE_UART_WAKE_LOOP_ON_RX` abgesichert),
  da der LIN-Bus-BREAK-Erkennungs-Task sie jederzeit benötigt

#### `uart_component_esp_idf.cpp` _(kritisch)_
- Präprozessor-Guard geändert: `USE_ESP_IDF` → `USE_ESP32_FRAMEWORK_ESP_IDF`
- `UART_SCLK_APB` → `UART_SCLK_DEFAULT` (ESP-IDF 5.x API-Änderung)
- `portTICK_RATE_MS` → `pdMS_TO_TICKS(20)` (aus ESP-IDF 5.x entfernt)
- Alle `lock_`-Mutex-take/give-Aufrufe entfernt (~12 Stellen)
- `static uint8_t next_uart_num` → `static uart_port_t next_uart_num = UART_NUM_0`
  (ESP-IDF 5.x: `uart_port_t` ist ein Scoped Enum, keine implizite `uint8_t`-Konvertierung)
- Postfix-`++` auf `uart_port_t` durch expliziten Cast ersetzt:
  `next_uart_num = (uart_port_t)(next_uart_num + 1)`
- `int available()` → `size_t available()`
- Implementierungen für `load_settings()`, `set_rx_full_threshold()`, `set_rx_timeout()` ergänzt

#### `truma_uart_component_esp_idf.h`
- Präprozessor-Guard geändert: `USE_ESP_IDF` → `USE_ESP32_FRAMEWORK_ESP_IDF`
- `get_hw_serial_number()` entfernt (wird jetzt von `IDFUARTComponent`-Basisklasse bereitgestellt)
- `get_uart_event_queue()` bleibt erhalten und gibt `&uart_event_queue_` zurück

#### `uart_component_esp32_arduino.h` / `.cpp`
- `int available()` → `size_t available()`
- `check_logger_conflict()`: `logger::global_logger->get_hw_serial()` mit
  `#if defined(USE_LOGGER) && !defined(USE_ESP32)` abgesichert — ESPHome 2026.1 hat
  `get_hw_serial()` aus `Logger` für ESP32 entfernt (Arduino auf ESP32 baut jetzt auf IDF auf)

#### `uart_component_rp2040.h` / `.cpp`
- `int available()` → `size_t available()`

#### `uart_component_esp8266.h` / `.cpp`
- `ESP8266UartComponent::available()`: `int` → `size_t`

---

### Geändert — `components/truma_inetbox/`

#### POSIX-Integer-Typ-Ersetzungen (alle 30 betroffenen Dateien)
- `u_int8_t` → `uint8_t`
- `u_int16_t` → `uint16_t`
- `u_int32_t` → `uint32_t`

Diese POSIX-Typen (`u_int*_t`) werden implizit von glibc-/BSD-libc-Headern bereitgestellt,
die die Arduino-Toolchain automatisch einbindet. Die ESP-IDF 5.x GCC-Toolchain stellt sie
**nicht** bereit, was zu 294 Kompilierfehlern in 30 Dateien führte.

Betroffene Dateien:
`LinBusProtocol.h`, `LinBusProtocol.cpp`, `LinBusListener.h`, `LinBusListener.cpp`,
`TrumaiNetBoxApp.h`, `TrumaiNetBoxApp.cpp`, `TrumaiNetBoxAppHeater.h/cpp`,
`TrumaiNetBoxAppAirconManual.h/cpp`, `TrumaiNetBoxAppAirconAuto.h/cpp`,
`TrumaiNetBoxAppClock.h/cpp`, `TrumaiNetBoxAppTimer.h/cpp`,
`TrumaStructs.h`, `TrumaEnums.h`, `TrumaStatusFrameBuilder.h`,
`TrumaStausFrameResponseStorage.h`, `helpers.h`, `helpers.cpp`,
`automation.h`, `time/TrumaTime.h` sowie sensor/number/select/climate-Unterkomponenten.

#### `LinBusListener_esp_idf.cpp`
- `#define QUEUE_WAIT_BLOCKING (portTickType) portMAX_DELAY`
  → `(TickType_t) portMAX_DELAY`
  (`portTickType` wurde in FreeRTOS 10 / ESP-IDF 5.x in `TickType_t` umbenannt)
- `uart_intr_config(uart_num, &uart_intr)` → `uart_intr_config((uart_port_t) uart_num, &uart_intr)`
  (ESP-IDF 5.x: `uart_intr_config` erfordert `uart_port_t`, keine implizite `uint8_t`-Konvertierung)

#### `LinBusListener_esp32_arduino.cpp`
- `#define QUEUE_WAIT_BLOCKING (portTickType) portMAX_DELAY`
  → `(TickType_t) portMAX_DELAY`
  (gleiche FreeRTOS-Umbenennung, betrifft auch Arduino auf ESP32, das auf ESP-IDF 5.x aufbaut)

---

### Hinzugefügt

- `test_compile.yaml` — minimale Testkonfiguration für ESP32-Arduino-Framework-Builds
- `test_compile_idf.yaml` — minimale Testkonfiguration für ESP32-ESP-IDF-Framework-Builds

---

### Hinweise

- ESPHome **2026.1.x existiert nicht** auf PyPI — die Versionsnummerierung springt von 2025.10.x
  direkt zu 2026.2.x.
- ESPHome 2026.1 hat `CORE.using_esp_idf` als veraltet markiert (nur Warnung; Verhaltensänderung in 2026.6).
  ESP32 Arduino baut nun offiziell auf ESP-IDF auf, sodass IDF-Features in beiden Frameworks verfügbar sind.
  Die `uart_component_esp32_arduino`-Komponente funktioniert weiterhin als benutzerdefinierter Override.
- Die Installation von ESPHome 2026.2.x in einem Python-venv erfordert zusätzlich das
  `fatfs-ng`-Paket (`pip install fatfs-ng`) als transitive PlatformIO-Abhängigkeit.
