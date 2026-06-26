# bambu-slack-notifier

ESP32 で Bambu Lab プリンター（A1 mini / P2S など）の MQTT を購読し、プリント状態の変化を Slack に通知するスケッチです。機種に依存せず、接続情報は `secrets.h` で切り替えます。

- プリント開始時に「合計推定時間」と「完了予定時刻」を通知
- 完了 / 一時停止・失敗 を通知

## 必要なもの

- ESP32 ボード
- Arduino IDE（ESP32 ボードサポート導入済み）
- ライブラリ: `PubSubClient`, `ArduinoJson`(v7)
- LAN モードを有効にした Bambu プリンター（IP / アクセスコード / シリアル番号）
- Slack の Incoming Webhook URL

## セットアップ

1. このフォルダ内の `secrets.example.h` を `secrets.h` にコピー
   ```sh
   cp secrets.example.h secrets.h
   ```
2. `secrets.h` を開き、自分の環境の値を記入
   | 項目 | 取得場所 |
   |---|---|
   | `WIFI_SSID` / `WIFI_PASSWORD` | 接続する Wi-Fi |
   | `MQTT_SERVER` | プリンター画面 設定 > ネットワーク の IP アドレス |
   | `MQTT_PASSWORD` | 同上の LAN Access Code |
   | `MQTT_TOPIC` | `device/<シリアル番号>/report` の形式で記入 |
   | `SLACK_WEBHOOK` | Slack App の Incoming Webhooks で発行した URL |
3. Arduino IDE でこのスケッチを開き、ESP32 に書き込み

`secrets.h` は `.gitignore` で除外されるためコミットされません。実値はここにだけ置いてください。

## 注意

- `mqtt_user` は Bambu 固定の `bblp` のためコード内に直書きしています。
- MQTT は TLS(8883) ＋ `setInsecure()` で接続します。
