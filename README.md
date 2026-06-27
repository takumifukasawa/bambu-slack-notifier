# bambu-slack-notifier

An ESP32 sketch that subscribes to a Bambu Lab printer's MQTT feed (A1 mini / P2S, etc.) and posts print-status changes to Slack. It is printer-model agnostic — connection details are switched via `secrets.h`.

- On print start: notifies the total estimated time and the expected finish time
- Notifies on completion / pause / failure

## Requirements

- An ESP32 board
- Arduino IDE (with ESP32 board support installed)
- Libraries: `PubSubClient`, `ArduinoJson` (v7)
- A Bambu printer with LAN mode enabled (IP / access code / serial number)
- A Slack Incoming Webhook URL

### Tested with

Other versions likely work too; this is just a known-good reference.

| Component | Version |
|---|---|
| Arduino IDE | 2.3.8 |
| ESP32 core (arduino-esp32) | 3.3.7 |
| ArduinoJson | 7.4.3 |
| PubSubClient | 2.8 |

## Arduino IDE setup

1. **Install ESP32 board support.** In Preferences, add this URL to *Additional boards manager URLs*:
   ```
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
   Then open Boards Manager and install **esp32 by Espressif**.
2. **Install the libraries** from Library Manager:
   - `PubSubClient` (Nick O'Leary)
   - `ArduinoJson` (Benoit Blanchon) — **v7 required** (v6 uses a different API)

   `WiFi.h`, `WiFiClientSecure.h`, `HTTPClient.h`, and `time.h` ship with the ESP32 core, so no extra install is needed.
3. **Select the board and port.** Tools > Board: pick your ESP32 (e.g. `ESP32 Dev Module`). Tools > Port: pick the connected port.

## Configure secrets

1. Copy `secrets.example.h` to `secrets.h` in this folder:
   ```sh
   cp secrets.example.h secrets.h
   ```
2. Open `secrets.h` and fill in the values for your environment:
   | Field | Where to find it |
   |---|---|
   | `WIFI_SSID` / `WIFI_PASSWORD` | The Wi-Fi network to connect to |
   | `MQTT_SERVER` | Printer screen: Settings > Network > IP address |
   | `MQTT_PASSWORD` | Same screen: the LAN Access Code |
   | `MQTT_TOPIC` | Use the form `device/<serial-number>/report` |
   | `SLACK_WEBHOOK` | The URL issued under Incoming Webhooks in your Slack App |

`secrets.h` is excluded by `.gitignore`, so it is never committed. Keep your real values only there.

## Build & upload

1. Open this sketch in the Arduino IDE (`secrets.h` must already exist, or the build fails with `WIFI_SSID undefined`).
2. Click Upload to flash the ESP32.
3. Open the Serial Monitor at **115200 baud** to watch the connection log.

## Troubleshooting

- **`WIFI_SSID` / `WIFI_PASSWORD` ... undefined**: you haven't created `secrets.h` yet (see *Configure secrets*).
- **ArduinoJson compile errors** (e.g. `StaticJsonDocument`): you're on v6. Install ArduinoJson v7.
- **Garbled Serial Monitor output**: the baud rate isn't 115200.
- **No MQTT connection**: enable LAN mode on the printer and double-check the IP, LAN Access Code, and serial number in `secrets.h`.

## Notes

- `mqtt_user` is hardcoded as `bblp` (fixed by Bambu).
- MQTT connects over TLS (port 8883) using `setInsecure()`.

## License

MIT — see [LICENSE](LICENSE).
