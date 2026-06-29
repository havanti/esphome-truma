# BLE Sniffing Guide — Truma Cooler

[🇩🇪 Deutsch](SNIFFING.md) | [🇬🇧 English](SNIFFING.en.md)

Help support more **Truma Cooler models** by capturing the BLE communication between the official **Truma app** and your cooler.

---

## Why?

The BLE protocol of the `truma_cooler` component was reverse-engineered **only from the C44 model** (single zone). Other models differ:

- **Multi-zone models** (e.g. the **C69** with two zones) carry extra fields in the frame — target/actual temperature and compressor state **per zone**.
- Other series may use **different service/characteristic UUIDs or GATT handles** — in which case connecting/subscribing already fails.

Without a capture of your model this can't be implemented. With a clean sniff plus your notes the protocol can be reconstructed and the component extended.

---

## Step 0 — Quick pre-check (5 minutes, no PC)

Before sniffing, temporarily add to your ESPHome configuration:

```yaml
logger:
  level: DEBUG
```

Let the ESP connect to the cooler once and send the serial logs (ESPHome → **Logs**). They reveal:

- whether the cooler **connects** at all,
- which **service UUID / GATT handles** service discovery returns,
- whether and which **frames** arrive.

Sometimes that alone is a useful first clue. Full multi-zone support still needs the app sniff below.

---

## Step 1 — Enable the Bluetooth HCI snoop log (Android)

You need an Android phone with the **Truma app** that controls your cooler normally.

1. Unlock **Developer options**: *Settings → About phone* → tap **"Build number"** 7×.
2. In Developer options, enable **"Enable Bluetooth HCI snoop log"**.
3. **Toggle Bluetooth off and on once** — otherwise the capture doesn't start.

> **Tip:** Disconnect other Bluetooth devices beforehand (headphones, smartwatch, car) so the capture stays clean and readable.

---

## Step 2 — Recording choreography

Now perform a **well-defined sequence** in the **Truma app** and write down **time + action** for each. This is the most important part — it's the only way to map bytes to functions.

Recommended order (skip the Zone 2 steps for single-zone models):

| # | Action | Note |
|---|--------|------|
| 1 | Connect app to cooler, leave idle ~10 s | baseline |
| 2 | **Zone 1 ON** | |
| 3 | **Zone 1 setpoint → +5 °C** | round value |
| 4 | **Zone 1 setpoint → −10 °C** | second round value |
| 5 | **Zone 2 ON** | multi-zone only |
| 6 | **Zone 2 setpoint → +5 °C** | multi-zone only |
| 7 | **Zone 2 setpoint → −10 °C** | multi-zone only |
| 8 | **Turbo/Boost ON**, wait briefly, **OFF** | if available |
| 9 | **Zone 2 OFF** | multi-zone only |
| 10 | **Zone 1 OFF** | |

Note the approximate time for each step (second precision is enough).

---

## Step 3 — Pull the log off the phone via ADB

### One-time setup on the PC

1. **Install ADB** (Android Platform Tools): <https://developer.android.com/tools/releases/platform-tools> — unzip, done (Windows/macOS/Linux).
2. On the phone, also enable **"USB debugging"** in Developer options.
3. Connect the phone to the PC via **USB cable**. On the phone confirm the **"Allow USB debugging?"** dialog with *OK*.
4. In a terminal/CMD inside the platform-tools folder, verify:
   ```
   adb devices
   ```
   Your device must appear with status `device` (not `unauthorized`).

### Method A — Bug report (reliable, recommended, no root)

```
adb bugreport bugreport.zip
```

Produces a ZIP. Inside, the log usually lives at

```
FS/data/misc/bluetooth/logs/btsnoop_hci.log
```

The path varies by Android version and vendor (e.g. also `FS/data/log/bt/` or directly in the ZIP root) — easiest is to search the extracted bug report for **`btsnoop`**. Sometimes there are several parts (`btsnoop_hci.log.*`) — send them all.

> **Fallback if no file shows up at all:** On some devices a vendor config sets the path (key `BtSnoopFileName`, e.g. in `/etc/bluetooth/bt_stack.conf`). Check there for the output location.

### Method B — Direct pull (legacy, very old Android versions only)

Very old devices (roughly before Android 9) wrote the snoop straight to internal storage:

```
adb pull /sdcard/btsnoop_hci.log
```

On newer Android versions this file practically never exists anymore → use **Method A**. "No such file or directory" → likewise Method A.

> **Important:** Disable the HCI snoop **only after** recording and grab the log **promptly** — another Bluetooth off/on overwrites it.

---

## Step 4 — What to send

Open an [issue](../../issues/new/choose) ("Report a Cooler model") and attach / provide:

- the **`btsnoop_hci.log`** (or several parts),
- your **action/time notes** from Step 2,
- the exact **model** (e.g. "Truma Cooler C69"),
- the **number of zones**,
- the **Truma app version**,
- optional: the ESPHome DEBUG logs from Step 0.

---

## Alternative — nRF Connect (no PC/ADB)

The **nRF Connect for Mobile** app (Android/iOS) as a simple entry point:

1. Connect nRF Connect to the cooler.
2. Under *Services*, note the **service/characteristic UUIDs + handles**.
3. Enable **"Notifications"** on the notify characteristic.
4. While toggling the zones, capture the logged values and export via the share function.

This does not replace a full app sniff (you don't see what the *app* sends), but it provides UUIDs/handles and the unsolicited status frames.

---

## Note on iOS

iOS has no easy HCI snoop log. Options:

- **nRF Connect for iOS** (see above), or
- Apple's **PacketLogger** (part of "Additional Tools for Xcode"; requires a Bluetooth debug profile installed on the iPhone via macOS).

The simplest path for a full app sniff remains an **Android phone**.

---

## Privacy

The `btsnoop_hci.log` contains the **entire BLE/HCI traffic of your phone** during the recording — including packets from other paired devices (headphones, smartwatch, car, etc.), with MAC addresses and sometimes device names. **Account, location or personal app data** is normally **not** included, and the capture is harmless for protocol analysis.

To keep the capture clean and minimal: **disconnect other BLE devices** beforehand (headphones, smartwatch) and record **only while operating the cooler**.
