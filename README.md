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

## Setup

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
3. Open this sketch in the Arduino IDE and flash it to the ESP32.

`secrets.h` is excluded by `.gitignore`, so it is never committed. Keep your real values only there.

## Notes

- `mqtt_user` is hardcoded as `bblp` (fixed by Bambu).
- MQTT connects over TLS (port 8883) using `setInsecure()`.
