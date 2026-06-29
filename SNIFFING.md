# BLE-Sniffing-Anleitung — Truma Cooler

[🇩🇪 Deutsch](SNIFFING.md) | [🇬🇧 English](SNIFFING.en.md)

Hilf mit, weitere **Truma-Cooler-Modelle** zu unterstützen, indem du einen Mitschnitt der BLE-Kommunikation zwischen der offiziellen **Truma-App** und deiner Kühlbox lieferst.

---

## Warum?

Das BLE-Protokoll der `truma_cooler`-Komponente wurde **ausschließlich am Modell C44** (eine Zone) reverse-engineered. Andere Modelle weichen ab:

- **Mehrzonen-Modelle** (z. B. **C69** mit zwei Zonen) haben zusätzliche Felder im Telegramm — Soll-/Ist-Temperatur und Kompressor-Status **je Zone**.
- Andere Baureihen nutzen evtl. **andere Service-/Characteristic-UUIDs oder GATT-Handles** — dann scheitert schon das Verbinden/Abonnieren.

Ohne Mitschnitt deines Modells lässt sich das nicht implementieren. Mit einem sauberen Sniff + deinen Notizen kann das Protokoll rekonstruiert und die Komponente erweitert werden.

---

## Schritt 0 — Schneller Vorab-Check (5 Minuten, ohne PC)

Bevor du sniffst: Trag in deine ESPHome-Konfiguration testweise ein

```yaml
logger:
  level: DEBUG
```

Lass den ESP einmal mit der Kühlbox verbinden und schick die seriellen Logs (ESPHome → **Logs**). Daraus ist ersichtlich:

- ob die Box überhaupt **verbindet**,
- welche **Service-UUID / GATT-Handles** die Service-Discovery liefert,
- ob und welche **Telegramme** (Frames) ankommen.

Manchmal reicht das schon als erster Anhaltspunkt. Für volle Mehrzonen-Unterstützung folgt aber der App-Sniff unten.

---

## Schritt 1 — Bluetooth-HCI-Snoop aktivieren (Android)

Du brauchst ein Android-Handy mit der **Truma-App**, das deine Kühlbox normal steuert.

1. **Entwickleroptionen** freischalten: *Einstellungen → Telefoninfo* → 7× auf **„Build-Nummer"** tippen.
2. In den Entwickleroptionen **„Bluetooth-HCI-Snoop-Log aktivieren"** einschalten.
3. **Bluetooth einmal aus- und wieder einschalten** — sonst greift die Aufzeichnung nicht.

---

## Schritt 2 — Aufzeichnungs-Choreografie

Jetzt mit der **Truma-App** eine **klar definierte Abfolge** durchführen und dabei **Uhrzeit + Aktion** notieren. Das ist der wichtigste Teil — nur so lassen sich die Bytes den Funktionen zuordnen.

Empfohlene Reihenfolge (bei einer Zone die Zone-2-Schritte weglassen):

| # | Aktion | Notiz |
|---|---|---|
| 1 | App mit Kühlbox verbinden, ~10 s im Ruhezustand lassen | Baseline |
| 2 | **Zone 1 EIN** | |
| 3 | **Zone 1 Solltemperatur → +5 °C** | runder Wert |
| 4 | **Zone 1 Solltemperatur → −10 °C** | zweiter runder Wert |
| 5 | **Zone 2 EIN** | nur Mehrzonen |
| 6 | **Zone 2 Solltemperatur → +5 °C** | nur Mehrzonen |
| 7 | **Zone 2 Solltemperatur → −10 °C** | nur Mehrzonen |
| 8 | **Turbo/Boost EIN**, kurz warten, **AUS** | falls vorhanden |
| 9 | **Zone 2 AUS** | nur Mehrzonen |
| 10 | **Zone 1 AUS** | |

Notiere zu jedem Schritt die ungefähre Uhrzeit (Sekundengenauigkeit reicht).

---

## Schritt 3 — Log per ADB vom Handy holen

### Einmalige Vorbereitung am PC

1. **ADB installieren** (Android Platform Tools): <https://developer.android.com/tools/releases/platform-tools> — ZIP entpacken, fertig (Windows/macOS/Linux).
2. Am Handy in den **Entwickleroptionen** zusätzlich **„USB-Debugging" aktivieren**.
3. Handy per **USB-Kabel** an den PC. Am Handy den Dialog **„USB-Debugging zulassen?"** mit *OK* bestätigen.
4. Im Terminal/CMD im Platform-Tools-Ordner prüfen:
   ```
   adb devices
   ```
   Dein Gerät muss mit Status `device` erscheinen (nicht `unauthorized`).

### Methode A — Bugreport (zuverlässig, empfohlen, kein Root)

```
adb bugreport bugreport.zip
```

Erzeugt ein ZIP. Darin liegt das Log unter

```
FS/data/misc/bluetooth/logs/btsnoop_hci.log
```

(manchmal mehrere `btsnoop_hci.log.*` — alle mitschicken). ZIP öffnen, Datei rausziehen.

### Methode B — Direkter Pull (schneller, nicht auf jedem Handy)

Manche Geräte schreiben den Snoop direkt auf den internen Speicher:

```
adb pull /sdcard/btsnoop_hci.log
```

Fehlermeldung „No such file or directory" → nimm Methode A.

> **Wichtig:** HCI-Snoop **erst nach** der Aufzeichnung deaktivieren und das Log **zeitnah** holen — bei erneutem Bluetooth-An/Aus wird es überschrieben.

---

## Schritt 4 — Was schicken

Öffne ein [Issue](../../issues/new/choose) („Cooler-Modell melden") und hänge an bzw. gib an:

- die **`btsnoop_hci.log`** (oder mehrere Teile),
- deine **Aktions-/Zeit-Notizen** aus Schritt 2,
- das genaue **Modell** (z. B. „Truma Cooler C69"),
- **Anzahl Zonen**,
- die **Version der Truma-App**,
- optional: die ESPHome-DEBUG-Logs aus Schritt 0.

---

## Alternative — nRF Connect (ohne PC/ADB)

Die App **nRF Connect for Mobile** (Android/iOS) als einfacher Einstieg:

1. nRF Connect mit der Kühlbox verbinden.
2. Unter *Services* die **Service-/Characteristic-UUIDs + Handles** notieren.
3. Bei der Notify-Characteristic **„Notifications" aktivieren**.
4. Beim Schalten der Zonen die geloggten Werte mitschneiden und per Teilen-Funktion exportieren.

Das ersetzt keinen vollständigen App-Sniff (man sieht nicht, was die *App* sendet), liefert aber UUIDs/Handles und die unaufgeforderten Status-Telegramme.

---

## Hinweis zu iOS

iOS bietet kein einfaches HCI-Snoop-Log. Möglichkeiten:

- **nRF Connect for iOS** (siehe oben), oder
- Apples **PacketLogger** (Teil der „Additional Tools for Xcode", erfordert ein per macOS installiertes Bluetooth-Debug-Profil auf dem iPhone).

Am unkompliziertesten bleibt ein **Android-Handy** für den vollständigen App-Sniff.

---

## Datenschutz

Die `btsnoop_hci.log` enthält ausschließlich Bluetooth-Verkehr — **keine Konto-, Standort- oder persönlichen Daten**. Sie kann unbesorgt geteilt werden. Wer ganz sicher gehen will, zeichnet nur während der Cooler-Bedienung auf und schließt vorher andere BLE-Geräte (Kopfhörer, Smartwatch).
