# Changelog

🇩🇪 Deutsch | [🇬🇧 English](CHANGELOG.en.md)

Alle wesentlichen Änderungen an diesem Projekt werden in dieser Datei dokumentiert.

Das Format basiert auf [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

---

## Kompatibilitätsstatus

### Zusammenfassung

Neu in 1.0.47: Die Ist-Temperatur erscheint in HA mit Zehnteln, das Wasser auch über MQTT (PR #40
von Marco). Seit 1.0.46 lässt sich die Klimaautomatik am CP Plus wieder aus HA setzen, laut Rückmeldung aus
Issue #32 funktioniert das. Seit 1.0.42 läuft auch die CP Plus C3 (Issue #29, PR #30 von Marco). Die
Lüfterauswahl an der Combi 6 Gas hat Marco ebenfalls bestätigt. `HEATER_HAS_ERROR` meldet jetzt auch
Warnungen. Der Landstrom-Sensor der Combi D aus 1.0.39 ist noch nicht bestätigt. AUTO-Anzeige und
Licht der Aventa sind bestätigt (Issue #28).

Getestet mit:
- ESPHome **2026.10.0** — ESP-IDF ✅

---


## [1.0.47] — 2026-10-27 — Ist-Temperatur in HA

### Behoben
- `truma_inetbox`: `ROOM` zeigt die Ist-Temperatur mit Zehnteln, `WATER` erscheint auch über MQTT
  (Issue #38, PR #40 von Marco). Von Marco getestet.

### Geändert
- `truma_inetbox`: `AIRCON` und `AIRCON_AUTO` ebenso. An einer Aventa noch nicht bestätigt.

## [1.0.46] — 2026-10-24 — Aventa: AUTO aus HA setzen (experimentell)

### Hinzugefügt
- `truma_inetbox`: Klima-Entität `AIRCON_AUTO` und Number `AIRCON_AUTO_TEMPERATURE` schalten die
  Klimaautomatik am CP Plus wieder ein und aus und setzen das Soll (18–25 °C). Das Format stammt aus
  Mitschnitten einer originalen iNet Box (Issue #32). Laut Rückmeldung aus dem Issue funktioniert das.

### Dokumentation
- README und Aventa-Beispiel um AUTO aus HA ergänzt, die Entitäten sind im Beispiel auskommentiert.
  `VENT_MODE` zeigt die Stufen 1–10 auch an einer weiteren Combi (Issue #32).

## [1.0.45] — 2026-10-15 — Aufräumen

### Geändert
- `truma_inetbox`: Die Schreibweisen `1.8kW`, `1,8 kW` für `watt` werden jetzt angenommen.
- `truma_inetbox`: `CP_PLUS_CONNECTED` bleibt nach einem Ausfall des CP Plus aus, bis wieder Anfragen
  kommen. Vorher sprang der Sensor etwa alle 71 Minuten kurz auf „verbunden“.
- `truma_inetbox`: Keine zweite Warnung mehr, wenn das CP Plus ein Update abholt, aber keins ansteht.
- Code aufgeräumt: benannte Konstanten statt Zahlen, der seit 1.0.41 ungenutzte Sendepfad der
  Klimaautomatik und andere tote Teile entfernt, Kommentare berichtigt. Kein Einfluss auf das Verhalten.

## [1.0.44] — 2026-10-15 — Folgebefehle, Robustheit

### Behoben
- `truma_inetbox`: Zwei Befehle kurz nacheinander (z. B. Temperatur setzen, dann Einschalten)
  überschreiben den ersten nicht mehr mit dem Einschalt-Wert. Raum, Wasser und Lüfterauswahl am
  Prüfaufbau bestätigt, die Aventa geht denselben Weg.
- `truma_inetbox`: Die Wasser-Number zeigt bei Boost 80 statt 200. Am Prüfaufbau bestätigt.

### Geändert
- `truma_inetbox`: Die Lüfterstufen im Raum-Klima wirken jetzt auch an einer Vario Heat (Night/Auto).
  Noch nicht bestätigt.
- `truma_inetbox`: Eine eigene, kürzere `options:`-Liste bei `select` wird bei der Prüfung der
  Konfiguration abgelehnt, vorher startete der ESP damit immer wieder neu.
- `truma_inetbox`: `observer_mode` lässt den LIN-Transceiver über `cs_pin` wach, sonst hört der ESP nichts.
- `truma_inetbox`: Timer-Zeiten bis 23:59; eine Stromstufe ohne Energiemix wird wie bei der Heizung ergänzt.
- `truma_inetbox`: Die Aventa-Number zeigt bei Aus 15 statt 0.
- `truma_inetbox`: Die Zeit-Plattform `time: platform: truma_inetbox` stellt die ESP-Uhr jetzt richtig.

### Entfernt
- `uart`: Die Option `debug:`, sie störte den LIN-Takt.

### Dokumentation
- README/Beispiele: Aventa-Beispiel ohne das entfernte AUTO-Setzen, Aktionen mit `DIESEL`/`NIGHT`/`AUTO`,
  veraltete Angaben (Mindestversion, Sensornamen, Links) bereinigt.

## [1.0.43] — 2026-10-14 — Heizen mit Temperatur, Uhr-Sync

### Behoben
- `truma_inetbox`: Heizen mit Temperatur in einem Aufruf (z. B. `climate.set_temperature` mit
  `hvac_mode: heat`) setzt jetzt die gewünschte Temperatur statt 5 °C (Wasser 40 °C). Am Prüfaufbau bestätigt.
- `truma_inetbox`: Die Uhrzeit wird erneut gesetzt, wenn HA beim ersten Versuch noch nicht verbunden
  war. Am Prüfaufbau bestätigt.

## [1.0.42] — 2026-10-14 — CP Plus C3

### Behoben
- `truma_inetbox`: CP Plus C3: Init und Befehle aus HA kommen auch bei schlafendem Panel an, die
  Uhrzeit wird dort nicht gesetzt. Von Marco bestätigt (Issue #29), danke an
  [Marco (@magior)](https://github.com/magior) für PR #30.

### Geändert
- `truma_inetbox`: `HEATER_HAS_ERROR` geht auch bei Warnungen an, z. B. W255 (kurz beim Start des
  CP Plus).
- `truma_inetbox`: Die Lüfterauswahl sendet an einer Combi 6 Gas Eco/High statt der Vario-Heat-Stufen.
  Von Marco bestätigt (PR #30).

### Dokumentation
- README: CP Plus C3, `HEATER_HAS_ERROR` und Dank an Marco.

## [1.0.41] — 2026-10-09 — Aventa: AUTO aus HA wieder entfernt

### Entfernt
- `truma_inetbox`: `AIRCON_AUTO` und `AIRCON_AUTO_TEMPERATURE` aus 1.0.40, der CP Plus lehnt den
  Befehl ab (Issue #28). Bitte aus der YAML löschen.

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
  Heizung. Experimentell, noch nicht bestätigt.

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
- `truma_inetbox`: `HEATING_DEMAND` (Wärmeanforderung, kein Brennerstatus) und `PID22_BYTE0`/`PID22_BYTE1`.
  Experimentell, nur Combi 4 (Issue #25).

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
- `truma_inetbox`: Falsche Zuordnung im VERBOSE-Log, betraf nur die Log-Ausgabe.

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
- `truma_cooler` C69: zwei Zonen mit eigener Soll- und Ist-Temperatur, ohne Turbo. Beispiel
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
- `truma_inetbox`: Gegen gleichzeitigen Zugriff abgesichert.

## [1.0.17] — 2026-04-29 — Truma Cooler: Robustheitsfixes

### Geändert
- `truma_cooler`: Code aufgeräumt und gegen gleichzeitigen Zugriff abgesichert.

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
- `truma_inetbox`: Unvollständige LIN-Daten werden verworfen.
- `truma_inetbox`: Der Betriebsstatus „ON 255“ wurde als „ON 25“ angezeigt.
- `truma_inetbox`: Ohne Zeitserver blieb die Uhr dauerhaft im Update-Zustand.
- `truma_cooler`: Die Solltemperatur wird auch übernommen, wenn gleichzeitig der Modus wechselt.
- `truma_inetbox`, `truma_cooler`: Gegen gleichzeitigen Zugriff abgesichert.

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
- `truma_inetbox`: Lange LIN-Nachrichten wurden verworfen, zu kurze nicht geprüft.
- `truma_inetbox`: Zeitvergleiche stimmen auch nach dem Zählerüberlauf (etwa alle 71 Minuten).
- `truma_inetbox`: Im Log fehlte die Prüfsumme.

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
