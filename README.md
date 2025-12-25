# AtomS3U BLE Keyboard

Turn your M5Stack AtomS3U into a wireless USB keyboard controller. Send keystrokes from your phone to your PC via Bluetooth.

## How It Works

```
┌─────────────┐      BLE       ┌─────────────┐    USB HID    ┌─────────────┐
│   Phone     │ ───────────────│   AtomS3U   │───────────────│     PC      │
│  (BLE App)  │    wireless    │  (ESP32-S3) │   keyboard    │             │
└─────────────┘                └─────────────┘               └─────────────┘
```

Your phone connects to the AtomS3U via Bluetooth Low Energy. The AtomS3U receives text and commands, then sends them to your PC as USB keyboard input. The PC sees a standard USB keyboard - no drivers or software needed.

## Use Cases

- Type on your PC from across the room
- Control presentations remotely
- Send keyboard shortcuts without touching your keyboard
- Accessibility - use your phone as an alternative input device
- Automation - trigger macros and shortcuts

## Hardware Required

- **M5Stack AtomS3U** (ESP32-S3 with USB-A connector)
- Any Android/iOS phone with BLE support

## Setup

### 1. Flash the Firmware

```bash
cd /home/eyal/atoms3u-ble-keyboard
pio run -t upload
```

If the device isn't detected, put it in bootloader mode:
1. Hold the button while plugging in USB
2. Release after 2 seconds

### 2. Connect Your Phone

Install **Serial Bluetooth Terminal** (Android) or **Bluefruit Connect** (iOS).

1. Open the app
2. Scan for BLE devices
3. Connect to **"AtomS3U-Keyboard"**
4. Start typing!

## Commands

### Text Input
Simply type any text and send - it will be typed on your PC followed by Enter.

### Special Keys

| Command | Key |
|---------|-----|
| `!ENTER` | Enter |
| `!TAB` | Tab |
| `!ESC` | Escape |
| `!BACKSPACE` | Backspace |
| `!SPACE` | Space |

### Arrow Keys

| Command | Key |
|---------|-----|
| `!UP` | Arrow Up |
| `!DOWN` | Arrow Down |
| `!LEFT` | Arrow Left |
| `!RIGHT` | Arrow Right |

### Keyboard Shortcuts

| Command | Action |
|---------|--------|
| `!CTRL+A` | Select All |
| `!CTRL+C` | Copy |
| `!CTRL+V` | Paste |
| `!CTRL+Z` | Undo |
| `!CTRL+T` | New Tab |
| `!CTRL+W` | Close Tab |
| `!CTRL+S` | Save |
| `!ALT+TAB` | Switch Window |
| `!SUPER` | Super/Windows Key |

### Function Keys

`!F1` through `!F12`

## Web App (Optional)

A web-based controller with buttons is included in the `web/` folder. It requires HTTPS hosting to use Web Bluetooth. Options:

- **GitHub Pages** - Push `web/` folder to a public repo
- **Cloudflare Pages** - Works with private repos
- **Netlify** - Free tier available
- **Local** - Use ngrok for temporary HTTPS tunnel

## Project Structure

```
atoms3u-ble-keyboard/
├── platformio.ini      # Build configuration
├── src/
│   └── main.cpp        # ESP32 firmware (BLE + USB HID)
├── web/
│   ├── index.html      # Web Bluetooth controller
│   └── manifest.json   # PWA manifest
└── README.md
```

## Technical Details

- **BLE Service**: Nordic UART Service (NUS)
  - UUID: `6e400001-b5a3-f393-e0a9-e50e24dcca9e`
- **USB**: HID Keyboard profile
- **Framework**: Arduino (ESP32)
- **Libraries**: ESP32 BLE, USB HID

## Troubleshooting

**Device not appearing as keyboard:**
- Ensure USB is connected directly to PC (not through a hub)
- Try a different USB port

**BLE not connecting:**
- Make sure the device is powered and running
- Check that Bluetooth is enabled on your phone
- Try restarting the AtomS3U

**Commands not working:**
- Ensure no extra spaces in commands
- Commands are case-sensitive (`!ENTER` not `!enter`)

## License

MIT
