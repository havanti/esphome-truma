# Changelog

🇩🇪 Deutsch | [🇬🇧 English](CHANGELOG.en.md)

Alle wesentlichen Änderungen an diesem Projekt werden in dieser Datei dokumentiert.

Das Format basiert auf [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

---

## Kompatibilitätsstatus

### Zusammenfassung

Das Setzen von AUTO aus HA (1.0.40) lehnt der CP Plus ab, es ist wieder entfernt. Der
Landstrom-Sensor der Combi D aus 1.0.39 ist noch nicht bestätigt. AUTO-Anzeige und Licht der Aventa
sind bestätigt (Issue #28).

Getestet mit:
- ESPHome **2026.9.1** — ESP-IDF ✅

---


## [1.0.41] — 2026-10-09 — Aventa: AUTO aus HA wieder entfernt

### Entfernt
- `truma_inetbox`: Klima-Entität `AIRCON_AUTO` und Number `AIRCON_AUTO_TEMPERATURE` aus 1.0.40. Der
  CP Plus lehnt den Befehl ab (Issue #28). Wer sie in der YAML hat, muss sie dort löschen, die
  AUTO-Anzeige bleibt.

### Dokumentation
- README und Aventa-Beispiel angepasst.

## [1.0.40] — 2026-10-09 — Aventa: AUTO aus HA setzen (experimentell)

### Hinzugefügt
- `truma_inetbox`: Klima-Entität `AIRCON_AUTO` und Number `AIRCON_AUTO_TEMPERATURE` schalten AUTO am
  CP Plus ein und aus und setzen das Soll. Vom CP Plus abgelehnt (Issue #28), in 1.0.41 entfernt.

### Dokumentation
- README und Aventa-Beispiel um AUTO aus HA ergänzt, die Entitäten sind im Beispiel auskommentiert.

## [1.0.39] — 2026-10-07 — Combi D: Landstrom an der Heizung (experimentell)

### Hinzugefügt
- `truma_inetbox`: Binärsensor `HEATER_MAINS_POWER` (nur Combi D) für 230 V Landstrom an der
  Heizung. Experimentell und für Automationen noch nicht geeignet, das Umschalten beim Einstecken
  ist noch nicht bestätigt.

### Geändert
- `truma_inetbox`: `AIRCON_AUTO_TARGET_TEMPERATURE` zeigt bei AUTO aus „Unbekannt“ statt 0 °C.
  Bestätigt an einer Aventa Compact Plus 2nd Gen (Issue #28).

### Dokumentation
- README: `HEATER_MAINS_POWER` und das Mitschneiden des Busverkehrs beschrieben.

## [1.0.38] — 2026-10-03 — Aventa: Licht und AUTO vom CP Plus

### Hinzugefügt
- `truma_inetbox`: Number `AIRCON_LIGHT` für das Licht der Aventa (0 = aus, 1–5). Laut Rückmeldung
  funktioniert das an einer Aventa der ersten Generation (Issue #28).
- `truma_inetbox`: Binärsensor `AIRCON_AUTO_ACTIVE` und Sensor `AIRCON_AUTO_TARGET_TEMPERATURE`
  zeigen AUTO am CP Plus an. Bestätigt an einer Aventa Compact Plus 2nd Gen (Issue #28).

### Behoben
- `truma_inetbox`: Befehle an die Aventa aus HA lassen das Licht unverändert, bisher wurde es dabei
  vermutlich ausgeschaltet. Laut Rückmeldung an einer Aventa der ersten Generation bestätigt.

### Dokumentation
- README: Aventa-Abschnitt und Aventa-Beispiel um Licht und AUTO ergänzt.

## [1.0.37] — 2026-09-30 — Code aufgeräumt

### Geändert
- `truma_inetbox`, `uart`: Code aufgeräumt, am Verhalten ändert sich nichts. Läuft seit 1.0.38 an
  einem CP Plus mit Aventa (Issue #28).

## [1.0.36] — 2026-09-30 — Webserver optional

### Geändert
- Beispiel-YAMLs: Der Webserver ist auskommentiert und optional. Ohne Anmeldung konnte ihn jeder im
  selben WLAN nutzen, das Beispiel enthält jetzt eine Anmeldung über `secrets.yaml`.

### Dokumentation
- README: neuer Abschnitt „Webserver (optional)“.

## [1.0.35] — 2026-09-29 — Warmwasser bei Aventa-Befehlen

### Behoben
- `truma_inetbox`: Befehle an die Aventa aus HA schalten das Warmwasser nicht mehr ab (Issue #16,
  #28). Bestätigt an einer Aventa Compact Plus 2nd Gen.

### Dokumentation
- README: Hinweis, dass AUTO am CP Plus nicht ausgewertet wird.

## [1.0.34] — 2026-09-29 — Kühlbox-Korrekturen, Absicherung der LIN-Kommunikation

### Geändert
- `truma_cooler`: Die Anmeldung für Statusmeldungen läuft über ESPHome statt über eigenen Code.
- `truma_inetbox`: Antworten an den CP Plus sind gegen gleichzeitigen Zugriff abgesichert. Selten
  waren vorher eine fehlerhafte Antwort oder ein Absturz möglich.
- `truma_inetbox`: Der LIN-Empfang rechnet seine Wartezeit auch beim Zählerüberlauf (etwa alle
  71 Minuten) richtig.
- `truma_inetbox`, `uart`: Fehler bei der UART-Einrichtung erscheinen zuverlässig im Log.

### Behoben
- `truma_cooler`: Erneutes „Kühlen“ an eine laufende Box schaltet beim C44 den Turbo nicht mehr ab.
- `truma_cooler`: Der Turbo-Schalter bleibt aus, wenn er bei ausgeschalteter Box betätigt wird.
- `truma_cooler`: Ein C44 mit Turbo wird als Turbo angezeigt, vorher standen Kompressor und Turbo
  auf aus.
- `truma_cooler`: Änderungen nach einem Befehl kommen nach etwa einer Sekunde in Home Assistant an
  statt nach bis zu einer Minute.

## [1.0.33] — 2026-09-28 — Aventa einschalten

### Behoben
- `truma_inetbox`: Die Aventa lässt sich aus HA jetzt auch mit dem CP Plus C.04.05.02 einschalten,
  der den Befehl bisher abgelehnt hat (Issue #28). Bestätigt an einer Aventa Compact Plus 2nd Gen.

### Dokumentation
- README: Hinweis, dass `VENT_MODE` an der Combi D6 E keine Werte liefert.

## [1.0.32] — 2026-09-24 — Lüfterstufe

### Hinzugefügt
- `truma_inetbox`: Sensor `VENT_MODE` zeigt die Lüfterstufe (0 aus, 1–10, 11 Heizlüfter Eco, 13
  Heizlüfter High). Nur Anzeige, an einer Combi 4 nachgeprüft (Issue #25).

### Dokumentation
- README: `HEATING_DEMAND` bleibt im reinen Boilerbetrieb aus.
- README: `PID22_BYTE0` ist die Versorgungsspannung, durch eine zweite Messung bestätigt.

## [1.0.31] — 2026-09-23 — Beispielkonfigurationen für ESPHome 2026.9.0

### Geändert
- Beispiel-YAMLs: Der API-Schlüssel kommt aus `!secret api_encryption_key`, ESPHome 2026.9.0 lehnt
  den leeren Schlüssel ab.
- Beispiel-YAMLs: OTA nutzt die API-Verschlüsselung statt eines Passworts. Umstieg siehe README,
  Abschnitt OTA.
- Beispiel-YAMLs: Kürzeres BLE-Scanfenster gegen WLAN-Abbrüche, `channel_colors` statt `rgb_order`.

### Dokumentation
- README: Voraussetzungen und OTA an die neuen Beispiele angepasst.

## [1.0.30] — 2026-09-23 — Heizanforderung (experimentell)

### Hinzugefügt
- `truma_inetbox`: Binärsensor `HEATING_DEMAND` und Sensoren `PID22_BYTE0`/`PID22_BYTE1`,
  experimentell und nur für die Combi 4 (Issue #25). `HEATING_DEMAND` zeigt, ob die Heizung Wärme
  anfordert, nicht ob der Brenner läuft.

## [1.0.29] — 2026-09-21 — Warnungen zu unbenutzten Funktionen

### Behoben
- `truma_inetbox`: Keine `-Wunused-function`-Warnungen mehr beim Build.

## [1.0.28] — 2026-09-21 — Formatwarnungen in der UART-Komponente

### Behoben
- `uart`: Keine `-Wformat`-Warnungen mehr beim Build für den ESP32-S3. Die geloggten Werte waren
  auch vorher richtig.

## [1.0.27] — 2026-09-19 — Actions in Triggern mit Argument

### Behoben
- `truma_inetbox`, `uart`: Die Actions lassen sich mit ESPHome 2026.9 auch in Triggern mit Argument
  nutzen, etwa in `api:`-Actions oder `on_value`. Vorher brach der Build dort ab.

## [1.0.26] — 2026-09-15 — Log-PID behoben, `OPERATING_STATUS` richtiggestellt

### Behoben
- `truma_inetbox`: Im VERBOSE-Log passten PID und Daten der LIN-Frames nicht zusammen. Betraf nur
  die Log-Ausgabe.

### Dokumentation
- README: `OPERATING_STATUS` ab 5 zeigt nicht den laufenden Brenner, die Werte hängen vom Modell ab
  (Issue #25).

## [1.0.25] — 2026-09-07 — Beispiel-YAMLs: `id` für Operating Status

### Dokumentation
- Heizungs-Beispiele: Der Sensor „Operating Status“ hat `id: operating_status` für Lambdas.
- README: Werte von `OPERATING_STATUS` erklärt, in 1.0.26 korrigiert.

## [1.0.24] — 2026-07-17 — Korrekturen aus Code-Audit

### Behoben
- `truma_inetbox`: „CP Plus verbunden“ meldete etwa alle 71 Minuten kurz fälschlich `false`.

## [1.0.23] — 2026-07-09 — UART-Startfehler behoben

### Behoben
- `uart`: Die UART-Komponente konnte je nach Build beim Start auf `FAILED` gehen, dann lief kein
  LIN-Verkehr („Cannot update Truma“). Die Ursache stand nur im seriellen Boot-Log (Issue #21).
- `truma_inetbox`: Build-Fehler mit `logger: level: VERBOSE` behoben (Issue #22).

### Geändert
- Die Log-Zeile `Component version:` zeigt wieder die richtige Version, sie stand seit 1.0.20 fest.

## [1.0.22] — 2026-07-03 — Truma Cooler C69: Master-Power-Schalter + Kompressor bestätigt

### Hinzugefügt
- `truma_cooler` C69: Schalter `power` schaltet die ganze Box ein und aus (Issue #18).

### Geändert
- `truma_cooler` C69: Die Zonen-Climates kühlen nur noch und setzen die Solltemperatur, Ein/Aus
  läuft über `power`. Bestehende C69-Konfigurationen brauchen den `power`-Schalter, siehe Beispiel.
- `truma_cooler` C69: Kompressor-Status an echter Hardware bestätigt (Issue #18).

## [1.0.21] — 2026-07-03 — Truma Cooler: C69 (zwei Zonen) via `model:`-Selektor

### Hinzugefügt
- `truma_cooler`: Auswahl `model:` (`c44` Standard, `c69`). C44-Konfigurationen ohne `model:` laufen
  unverändert.
- `truma_cooler` C69: Zwei Zonen mit eigener Soll- und Ist-Temperatur, Kompressor- und Gerätestatus.
  Ein/Aus gilt für die ganze Box, Turbo ist nicht umgesetzt. Beispiel
  `ESP32_truma_cooler_C69_example.yaml`.

## [1.0.20] — 2026-05-12 — Truma Cooler: Absicherung und Zustand nach Neustart

### Hinzugefügt
- `truma_cooler`: Modus und Solltemperatur bleiben über einen Neustart erhalten, Turbo und
  Gerätestatus starten mit „aus“ statt „unbekannt“.

### Geändert
- `truma_cooler`: BLE-Ereignisse ändern Entitäten nur noch aus der Hauptschleife.

### Behoben
- `truma_cooler`: Schnelles Ein- und Ausschalten schickt keinen überflüssigen Turbo-Aus-Befehl mehr.

## [1.0.19] — 2026-05-12 — RP2040-Support entfernt

### Entfernt
- Unterstützung für den Raspberry Pi Pico (RP2040), das Repo ist nur noch für ESP-IDF.

## [1.0.18] — 2026-04-29 — Thread-Sicherheit Heizung/Klima/Uhr

### Geändert
- `truma_inetbox`: Statusflags zwischen LIN-Task und Hauptschleife sind gegen gleichzeitigen Zugriff
  abgesichert.

## [1.0.17] — 2026-04-29 — Truma Cooler: Robustheitsfixes

### Geändert
- `truma_cooler`: Code aufgeräumt, Zugriffe zwischen BLE- und App-Task abgesichert.

### Behoben
- `truma_cooler`: Turbo-Befehle bei ausgeschalteter Box werden ignoriert.
- `truma_cooler`: Nach dem Verbinden zeigt das Climate „aus“ statt „unbekannt“.
- `truma_cooler`: Ein Verbindungsabbruch lässt keinen offenen Turbo-Reset mehr zurück.

## [1.0.16] — 2026-04-26 — Versionsanzeige im Webinterface

### Hinzugefügt
- Text-Sensor zeigt die Komponentenversion in Home Assistant und im Webinterface, alle
  Heizungs-Beispiele enthalten ihn. Die Version steht auch im Start-Log.

## [1.0.15] — 2026-04-23 — Sicherheits- und Robustheitsfixes

### Behoben
- `truma_inetbox`: Unvollständige LIN-Frames werden verworfen, volle Warteschlangen im Log gemeldet.
- `truma_inetbox`: Der Betriebsstatus „ON 255“ wurde als „ON 25“ angezeigt.
- `truma_inetbox`: Ohne Zeitserver blieb die Uhr dauerhaft im Update-Zustand.
- `truma_cooler`: Die Solltemperatur wird auch übernommen, wenn gleichzeitig der Modus wechselt.
- `truma_inetbox`, `truma_cooler`: Zugriffe zwischen Tasks abgesichert.

## [1.0.14] — 2026-04-21 — Truma Cooler C(XX) Integration

### Hinzugefügt
- Neue Komponente `truma_cooler` für die Truma Cooler C(XX) über BLE: Climate (−22 bis +10 °C),
  Innen- und Außentemperatur, Kompressor, Turbo, Geräte- und Verbindungsstatus.
- Beispiel `ESP32_truma_cooler_example.yaml` (M5Stack Atom Lite), parallel als Bluetooth Proxy
  nutzbar.

## [1.0.13] — 2026-04-20 — Truma Aventa Gen 2 Klimaanlage

### Hinzugefügt
- Truma Aventa Gen 2 über denselben LIN-Bus wie die Heizung: Climate `AIRCON`, Selects `AIRCON_MODE`
  und `AIRCON_VENT_MODE`, Number `AIRCON_MANUAL_TEMPERATURE`. Beispiel
  `ESP32-S3_truma_Aventa_example.yaml`.

## [1.0.12] — 2026-04-19 — LIN-Korrekturen

### Behoben
- `truma_inetbox`: LIN-Nachrichten über 255 Byte wurden verworfen.
- `truma_inetbox`: Eingehende Frames werden vor der Auswertung auf ihre Länge geprüft.
- `truma_inetbox`: Zeitvergleiche stimmen auch nach dem Zählerüberlauf (etwa alle 71 Minuten),
  Zugriffe zwischen Tasks abgesichert.
- `truma_inetbox`: Im Log fehlte das CRC-Byte.

## [1.0.11] — 2026-04-17 — ESPHome 2026.4.0 Kompatibilität

### Behoben
- `truma_inetbox`: Build-Fehler mit ESPHome 2026.4 behoben.
- Beispiel-YAMLs: Sensoren mit demselben Namen wie eine Number oder ein Select brachten mit ESPHome
  2026.4.0 die HA-Integration zum Absturz. `Target Room Temperature`, `Target Water Temperature`,
  `Electric Power Level` und `Energy Mix` heißen als Sensor jetzt mit Suffix „Status“. Nach dem
  Flashen die alten Sensoren in HA löschen und Dashboards und Automationen auf die neuen umstellen.

### Dokumentation
- Hardware-Dokumentation für den Nachbau erweitert.

## [1.0.10] — 2026-04-11 — Weitere Aufräumarbeiten

### Geändert
- `truma_inetbox`: Code aufgeräumt. Das Heizungsmodul loggt seinen Zustand auf DEBUG, Fehlercodes
  auf WARN.

## [1.0.9] — 2026-04-02 — Aufräumen

### Geändert
- Code aufgeräumt, keine Verhaltensänderung.

## [1.0.8] — 2026-03-30 — Codequalität

### Behoben
- `truma_inetbox`: Energiemix und elektrische Leistungsstufe wurden in der Antwort an den CP Plus
  falsch gesetzt.

### Geändert
- Tippfehler und Kommentare bereinigt.

### Dokumentation
- README nennt den Fork von @kamahat.

## [1.0.7] — 2026-03-28 — Kleinere Verbesserungen

### Behoben
- „LIN CRC error on SID“ erscheint nur noch im VERBOSE-Log, es ist kein echter Fehler (Vorschlag von
  @kamahat).

### Dokumentation
- `min_version: 2026.3.1` in allen Beispielen, CONTRIBUTING-Dateien ergänzt.

## [1.0.6] — 2026-03-27 — Robustheit

### Behoben
- `truma_inetbox`: Absturz beim Start auf Dual-Core-ESP32 behoben. Kommt der UART-Treiber nicht
  hoch, steht nach 5 s eine klare Meldung im Log.

## [1.0.5] — 2026-03-27 — Verbesserungen

### Geändert
- Beispiel-YAMLs: `refresh: 24h` statt `0s`, Alternativen als Kommentar.

## [1.0.4] — 2026-03-23 — Fehlerbehebungen

### Behoben
- `uart`: Prüfung der Rohdaten für `uart.write` korrigiert.
- README: Veralteter Name der Diesel-Beispieldatei korrigiert.

## [1.0.3] — 2026-03-22 — OTA, Aufräumen

### Hinzugefügt
- OTA in allen WLAN-Beispielen, dazu ein Abschnitt im README.

### Entfernt
- Ethernet-Beispiele von WomoLin und das Verzeichnis `examples/`.

## [1.0.2] — 2026-03-19 — Beispielkonfigurationen und Dokumentation

### Hinzugefügt
- Gas-Beispiele für ESP32 und ESP32-S3.

### Geändert
- Die Diesel-Beispiele heißen jetzt `*_6DE_Diesel_example.yaml`.
- Actions an ESPHome 2026.3.0 angepasst.

### Dokumentation
- README: Auswahl der Beispiele nach Energiequelle und Hardware, Hinweise zur Combi 4 und zu
  Diesel-Modellen ohne Eberspächer-Brenner.

## [1.0.1] — 2026-03-14 — ESPHome 2026.6 Kompatibilität

### Geändert
- Veraltete ESPHome-APIs ersetzt, damit die Komponenten auch mit ESPHome 2026.6 bauen.

## [1.0.0] — 2026-03-02 — ESPHome 2025.8+ / 2026.3.x Kompatibilität

### Hinzugefügt
- Test-Konfigurationen `test_compile.yaml` und `test_compile_idf.yaml`.

### Geändert
- `uart`: An ESPHome 2025.8+ und ESP-IDF 5.x angepasst. Die eigene UART-Komponente bleibt nötig,
  weil die LIN-Erkennung die Event-Queue braucht.
- `truma_inetbox`: Baut mit ESP-IDF 5.x.
