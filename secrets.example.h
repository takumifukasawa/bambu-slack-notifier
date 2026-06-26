// secrets.example.h
// Copy this file to secrets.h and fill in the values for your environment:
//   cp secrets.example.h secrets.h
// secrets.h is excluded by .gitignore, so it is never committed.
#pragma once

// --- Wi-Fi ---
#define WIFI_SSID      "YOUR_WIFI_SSID"
#define WIFI_PASSWORD  "YOUR_WIFI_PASSWORD"

// --- Bambu printer (LAN) ---
// Printer IP address (printer screen: Settings > Network)
#define MQTT_SERVER    "192.168.x.x"
// LAN Access Code (printer screen: Settings > Network)
#define MQTT_PASSWORD  "XXXXXXXX"
// Subscribe topic. Fill in the serial number (SN): device/<SN>/report
#define MQTT_TOPIC     "device/YOUR_PRINTER_SERIAL/report"

// --- Slack ---
// Incoming Webhook URL (issued under Incoming Webhooks in your Slack App)
#define SLACK_WEBHOOK  "https://hooks.slack.com/services/XXXX/XXXX/XXXX"
