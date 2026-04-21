# Hardware-Aufbau: Truma LIN-Bus Adapter

🇩🇪 Deutsch | [🇬🇧 English](README.en.md) | [🇫🇷 Français](README.fr.md)

Dieses Dokument beschreibt den Aufbau des Hardware-Adapters, der einen ESP32-S3
über den LIN-Bus mit einer Truma Combi-Heizung verbindet und die Steuerung via
Home Assistant im Wohnmobil ermöglicht.

---

## Benötigte Teile

| Nr. | Komponente | Modell / Hinweis |
|---|---|---|
| 1 | Mikrocontroller | z. B. Waveshare ESP32-S3-DEV-KIT-N8R8 |
| 2 | LIN-Transceiver | z. B. TJA1020-Modul (FST T151) oder WomoLIN Board v2 |
| 3 | Spannungswandler | z. B. DC/DC-Konverter 12V → 5V, mind. 500 mA |
| 4 | Anschlusskabel | RJ12-Stecker 6-polig (Truma-Port) |
| 5 | Kabel | Litze ≥ 0,5 mm², verschiedene Farben empfohlen |
| 6 | Sicherung | 1A Flachsicherung inkl. Halter |

---

## Sicherheitshinweise

> ⚠️ **Plus (+) und Minus (−) niemals vertauschen** — das beschädigt ESP32 und
> LIN-Transceiver sofort und dauerhaft.

> ⚠️ **Der LIN-Transceiver benötigt 12V direkt vom Bordnetz** — nicht vom 5V-Ausgang
> des DC/DC-Konverters.

> ✅ **Erst alle Verbindungen herstellen, dann Strom einschalten.**

> ✅ **1A-Sicherung** in der 12V Plus-Leitung empfohlen, möglichst nahe an der Batterie.

---

## Verdrahtungsplan

<img src="../pics/Wiring.png" width="700" alt="Montage- &amp; Verdrahtungsplan">

---

## Schritt-für-Schritt

### Schritt 1 — 12V Bordnetz → DC/DC-Konverter

Der Konverter wandelt die 12V Bordspannung auf die 5V um, die der ESP32-S3 benötigt.

| Kabel | Von | Zu |
|---|---|---|
| Rot | 12V Bordnetz (+) | DC/DC Eingang (+) |
| Schwarz | 12V Bordnetz (−) | DC/DC Eingang (−) |

> Sicherung in die rote Plus-Leitung einschleifen.

---

### Schritt 2 — DC/DC-Konverter → ESP32-S3

| Kabel | Von | Zu |
|---|---|---|
| Rot | DC/DC Ausgang (+) | ESP32-S3 **5V-Pin** |
| Schwarz | DC/DC Ausgang (−) | ESP32-S3 **GND-Pin** |

> Den **5V-Pin** verwenden — nicht den 3,3V-Pin!

---

### Schritt 3 — ESP32-S3 → LIN-Transceiver

| Kabel | ESP32-S3 | LIN-Transceiver |
|---|---|---|
| Grün | GPIO18 | TX |
| Grün | GPIO8 | RX |
| Schwarz | GND | GND |

> TX und RX aus Sicht des ESP32: **TX = senden**, **RX = empfangen**.

---

### Schritt 4 — LIN-Transceiver → RJ12 (Truma-Port)

<img src="../pics/PIN.png" width="300" alt="RJ12 Pin-Belegung">

| Kabel | Von | Zu |
|---|---|---|
| Orange | TJA1020 LIN | RJ12 **Pin 3** |
| Blau | TJA1020 GND | RJ12 **Pin 5** |

---

### Schritt 5 — 12V Bordnetz → LIN-Transceiver (zuletzt!)

| Kabel | Von | Zu |
|---|---|---|
| Rot | 12V Bordnetz (+) | TJA1020 **12V-Eingang** |

> Zweite **1A-Sicherung** in die rote Plus-Leitung zum TJA1020 einschleifen.

> Die genaue Bezeichnung des 12V-Anschlusses variiert je nach Boardtyp
> (FST T151, WomoLIN Board v2 o. ä.) — bitte Datenblatt des jeweiligen Moduls beachten.

---

## Alle Verbindungen im Überblick

| # | Von | Zu | Spannung |
|---|---|---|---|
| 1 | 12V Bordnetz (+) | DC/DC Eingang (+) | 12V |
| 2 | 12V Bordnetz (−) | DC/DC Eingang (−) | GND |
| 3 | DC/DC Ausgang (+) | ESP32-S3 5V-Pin | 5V |
| 4 | DC/DC Ausgang (−) | ESP32-S3 GND-Pin | GND |
| 5 | ESP32-S3 GPIO18 | TJA1020 TX | 3,3V |
| 6 | TJA1020 RX | ESP32-S3 GPIO8 | 3,3V |
| 7 | ESP32-S3 GND | TJA1020 GND | GND |
| 8 | 12V Bordnetz (+) | TJA1020 12V-Eingang | 12V |
| 9 | TJA1020 LIN | RJ12 Pin 3 | LIN-Signal |
| 10 | TJA1020 GND | RJ12 Pin 5 | GND |

---

## Vor dem ersten Einschalten

- [ ] Plus und Minus überall korrekt gepolt?
- [ ] Kein Kabel eingeklemmt oder beschädigt?
- [ ] RJ12-Stecker fest eingesteckt?
- [ ] Sicherung eingesetzt?

---

## Fehlersuche

| Problem | Mögliche Ursache | Lösung |
|---|---|---|
| ESP32 erscheint nicht im WLAN | Falsche WLAN-Daten in der Firmware | Firmware mit korrekten Zugangsdaten neu flashen |
| Kein LIN-Signal | 12V fehlt am LIN-Transceiver | Verbindung #8 und Sicherung prüfen |
| ESP32 startet nicht | Keine 5V-Versorgung | Ausgangsspannung des DC/DC-Konverters messen |
| Truma reagiert nicht | RJ12 Pin-Belegung falsch | Pin 3 (LIN) und Pin 5 (GND) am RJ12-Stecker prüfen |

---

## ESPHome-Konfiguration

Die passenden Beispiel-YAMLs sind im Repository verfügbar:
**[codeberg.org/havanti/esphome-truma](https://codeberg.org/havanti/esphome-truma)**

Für den ESP32-S3 die passende Variante wählen:

| Heizungstyp | Beispieldatei |
|---|---|
| Truma Combi 4/6 kW Gas | [`ESP32-S3_truma_4-6_Gas_example.yaml`](../ESP32-S3_truma_4-6_Gas_example.yaml) |
| Truma Combi 6 kW Diesel (6DE) | [`ESP32-S3_truma_6DE_Diesel_example.yaml`](../ESP32-S3_truma_6DE_Diesel_example.yaml) |

**Onboard-LED (Waveshare ESP32-S3):** Der Waveshare ESP32-S3-DEV-KIT-N8R8 besitzt eine WS2812-RGB-LED auf GPIO38. Die Beispiel-YAMLs nutzen diese als Statusanzeige — wenn Verkabelung und Konfiguration korrekt sind, blinkt die LED **grün** (CP Plus verbunden) bzw. **blau** (LIN-Daten werden gesendet).

**Mindestversion:** ESPHome 2026.3.1

**CP Plus vs. iNet Box:** Die Beispiel-YAMLs verwenden `lin_checksum: VERSION_2` (für CP Plus).
Bei einer älteren iNet Box ggf. auf `VERSION_1` wechseln.
