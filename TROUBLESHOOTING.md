# Fehlersuche — Truma iNet Box (LIN-Bus)

🇩🇪 Deutsch | [🇬🇧 English](TROUBLESHOOTING.en.md)

Diese Anleitung deckt die häufigsten Probleme bei der Inbetriebnahme der Heizungs-Komponente (`truma_inetbox`) ab. Für die reine Hardware-Verdrahtung siehe die [Fehlersuche im Hardware-Guide](hardware/README.md#fehlersuche); für die Kühlbox siehe die [Sniffing-Anleitung](SNIFFING.md).

---

## Grundregel: Immer zuerst das serielle DEBUG-Log

Fast jede Diagnose steht und fällt mit einem **seriellen** Boot-Log:

```yaml
logger:
  level: DEBUG
```

Dann den ESP per USB anschließen und ~30 Sekunden ab dem Einschalten mitschneiden:

```bash
esphome logs <config>.yaml
```

**Wichtig:** Fehler, die in `setup()` passieren (z. B. UART-Initialisierung), erscheinen **nur im seriellen Log**. Logs über die API (Home-Assistant-Add-on, `esphome logs` übers Netzwerk) starten erst nach dem WLAN-Verbindungsaufbau und zeigen dann nur noch `Component ... marked FAILED` — ohne den Grund.

---

## Symptom-Übersicht

| Symptom | Wahrscheinliche Ursache | Abschnitt |
|---|---|---|
| CP Plus zeigt die iNet Box nie „grün" / verbindet nicht | Checksum, fehlender Slave-Slot oder Verdrahtung | [1](#1-cp-plus-verbindet-nicht-nie-grün) |
| Log voller `Cannot update Truma`-Warnungen | UART/LIN-Bus tot | [2](#2-cannot-update-truma--uart-marked-failed) |
| `Component uart marked FAILED` beim Start | Bug in Versionen < 1.0.23 | [2](#2-cannot-update-truma--uart-marked-failed) |
| Boot-Schleife mit `flash read err, 1000` | Warm-Reset-Artefakt, kein Hardware-Defekt | [3](#3-boot-schleife-mit-flash-read-err-1000) |
| Crash-Schleife beim Start (esp32dev-Config) | ESP32-PICO-D4: GPIO16/17 sind Flash-Pins | [4](#4-esp32-pico-d4-gpio1617-nicht-nutzbar) |
| Home Assistant verliert die Verbindung / `aioesphomeapi`-Fehler | Entity-Namenskollision | [5](#5-entity-namenskollision-api-absturz) |

---

## 1. CP Plus verbindet nicht (nie „grün")

Die drei häufigsten Ursachen, in dieser Reihenfolge prüfen:

### 1a. `lin_checksum` muss `VERSION_2` sein

`VERSION_2` (LIN 2.x enhanced) ist der Default der Komponente und in allen Beispiel-YAMLs gesetzt. Mit `VERSION_1` **verwirft das CP Plus sämtliche Frames des ESP** — es kommt nie eine Verbindung zustande. Wer den Wert geändert hat: zurückstellen.

### 1b. CP Plus braucht einen freien Slave-Slot (Neu-Anlernen)

Das CP Plus registriert LIN-Slaves **nur während seiner Initialisierung**. War beim letzten Anlernen keine funktionierende iNet Box am Bus, ist kein Slot reserviert — der ESP kann sich dann nicht registrieren, egal wie korrekt er sendet.

**Lösung — Slots leeren und neu anlernen** (gilt insbesondere bei der **ersten Inbetriebnahme** des ESP32 als LIN-Slave):

1. **Alle Geräte vom CP Plus trennen** (Heizung, iNet Box/ESP, …)
2. CP-Plus-Initialisierung durchlaufen lassen → die Slots sind danach leer
3. **Alles wieder anschließen** (inkl. ESP mit korrekter `VERSION_2`-Konfiguration)
4. Initialisierung **erneut** durchlaufen lassen

Danach sollten alle Geräte am CP Plus zu sehen sein — sofern alles korrekt angeschlossen ist. Die Initialisierung dauert nur wenige Sekunden und darf nicht hängen bleiben.

### 1c. Verdrahtung prüfen

- LIN-Transceiver (TJA1020-Modul, FST T151, WomoLIN v2, …) ist Pflicht — der ESP kann LIN nicht direkt sprechen.
- Transceiver braucht **12 V direkt vom Bordnetz** (nicht die 5 V des ESP).
- RJ12: LIN → **Pin 3**, GND → **Pin 5**. Details im [Hardware-Guide](hardware/README.md).
- UART-Pins: ESP32 (`esp32dev`) `tx: 17` / `rx: 16`, ESP32-**S3** `tx: 18` / `rx: 8`. TX/RX zwischen ESP und Transceiver **nicht kreuzen**.

**Diagnose per DEBUG-Log:** Kommen **gar keine LIN-Frames** an → Verkabelung/Pins/12V. Kommen Frames an, aber keine Verbindung → Checksum (1a) oder Slot (1b).

### 1d. Wenn alles stimmt und es trotzdem nicht klappt: CP-Plus-Firmware

Ältere CP-Plus-Firmwarestände unterstützen die iNet Box nicht. Wenn Verdrahtung, `VERSION_2` und Neu-Anlernen nachweislich korrekt sind und die Initialisierung trotzdem nie einen Slot vergibt, kann die Bedienteil-Firmware die Ursache sein. Firmware-Updates gibt es nur über Truma-Servicepartner.

---

## 2. `Cannot update Truma` / UART `marked FAILED`

Dauerhafte `Cannot update Truma`-Warnungen bedeuten: Die App-Logik läuft, aber der LIN-Bus liefert nichts — meist weil die UART-Komponente beim Start gescheitert ist.

- **Versionen vor 1.0.23:** Bug — die UART-Konfiguration wurde uninitialisiert angelegt; je nach Build schlug `uart_param_config()` fehl und die UART-Komponente wurde beim Start `marked FAILED`. Das Verhalten war build-abhängig (identischer Code konnte in einem Build laufen, im nächsten nicht). **Fix: auf ≥ 1.0.23 aktualisieren.**
- Der eigentliche Fehlergrund (`uart_param_config failed: ...`) steht **nur im seriellen Boot-Log** — siehe [Grundregel](#grundregel-immer-zuerst-das-serielle-debug-log).
- `setup() finished successfully!` im Log schließt einen toten UART **nicht** aus.

---

## 3. Boot-Schleife mit `flash read err, 1000`

Sieht dramatisch aus, ist aber in der Regel **kein defekter Flash** und kein Spannungsproblem: Die Meldung ist ein Artefakt schneller Warm-Resets (z. B. Watchdog-Resets nach einem Absturz in `setup()`).

**Vorgehen:** Den ESP stromlos machen und einen **kalten Power-On** seriell mitschneiden — der zeigt den echten Fehler. Das vollständige Roh-Log lesen, nicht nur nach Schlagwörtern suchen. Wenn `esphome upload`/esptool fehlerfrei flasht und verifiziert, ist der Flash gesund — dann steckt der Fehler im Setup (z. B. Pin-Konflikt, siehe unten).

---

## 4. ESP32-PICO-D4: GPIO16/17 nicht nutzbar

Beim **ESP32-PICO-D4** sind GPIO16/17 intern mit dem eingebetteten Flash verbunden. Die Standard-UART-Pins der `esp32dev`-Beispiele (`tx: 17` / `rx: 16`) führen dort zu einer Crash-Schleife in `setup()`.

**Lösung:** Ein Board mit **WROOM- oder WROVER-Modul** verwenden (GPIO16/17 frei) — darauf sind die Beispiele verifiziert. Alternativ auf ESP32-S3 ausweichen (`tx: 18` / `rx: 8`).

---

## 5. Entity-Namenskollision (API-Absturz)

Ein `sensor:` und ein `number:`/`select:` mit **identischem Namen** erzeugen denselben Hash-Key — `aioesphomeapi` (Home Assistant) stürzt beim Verbinden ab.

**Lösung:** Sensor-Readbacks mit dem Suffix `Status` benennen (z. B. `Target Room Temperature Status`), wie in den Beispiel-YAMLs ab Version 1.0.20.

---

## Checkliste vor dem Issue

Wenn nichts hilft: [Bug melden](https://github.com/havanti/esphome-truma/issues/new/choose) — mit diesen Angaben geht es am schnellsten:

- [ ] Komponenten-Version (Release-Tag, z. B. `v1.0.23`) und ESPHome-Version
- [ ] Board (genaues Modul: WROOM / WROVER / PICO-D4 / S3)
- [ ] CP-Plus-Modell und Software-Version (steht auf der Platine des CP Plus, meist als Aufkleber)
- [ ] Verwendete YAML (Secrets entfernt) — idealerweise ein unverändertes Beispiel
- [ ] **Serielles** DEBUG-Boot-Log ab dem Einschalten (~30 s)
