// secrets.example.h
// このファイルを secrets.h にコピーして、自分の環境の値を埋めてください。
//   cp secrets.example.h secrets.h
// secrets.h は .gitignore で除外されるのでコミットされません。
#pragma once

// --- Wi-Fi ---
#define WIFI_SSID      "YOUR_WIFI_SSID"
#define WIFI_PASSWORD  "YOUR_WIFI_PASSWORD"

// --- Bambu プリンター (LAN) ---
// プリンターのIPアドレス（プリンター画面 設定 > ネットワーク で確認）
#define MQTT_SERVER    "192.168.x.x"
// LAN Access Code（プリンター画面 設定 > ネットワーク のアクセスコード）
#define MQTT_PASSWORD  "XXXXXXXX"
// 購読トピック。シリアル番号(SN)を埋める: device/<SN>/report
#define MQTT_TOPIC     "device/YOUR_PRINTER_SERIAL/report"

// --- Slack ---
// Incoming Webhook URL（Slack App の Incoming Webhooks で発行）
#define SLACK_WEBHOOK  "https://hooks.slack.com/services/XXXX/XXXX/XXXX"
