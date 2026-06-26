#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <time.h>
#include "secrets.h"

// ==========================================
// 設定値は secrets.h に記述（secrets.example.h をコピーして作成）
// ==========================================
const char* ssid = WIFI_SSID;          // Wi-Fiの名前
const char* password = WIFI_PASSWORD;  // Wi-Fiのパスワード

const char* mqtt_server = MQTT_SERVER;      // プリンターのローカルIPアドレス
const char* mqtt_user = "bblp";               // Bambuは「bblp」固定
const char* mqtt_password = MQTT_PASSWORD; // プリンター画面のLAN Access Code
const char* mqtt_topic = MQTT_TOPIC; // device/<シリアル番号>/report

const char* slack_webhook_url = SLACK_WEBHOOK; // SlackのWebhook URL
// ==========================================

WiFiClientSecure espClient;
PubSubClient client(espClient);

String currentState = "UNKNOWN";
bool isStartNotified = false;

const long gmtOffset_sec = 32400; // 日本標準時 (9時間 * 3600秒)
const int daylightOffset_sec = 0;

// Slackへメッセージを送る関数
void sendSlackMessage(String message) {
  // for debug
  // Serial.println("★Slack通知テスト送信予定の内容:");
  // Serial.println(message);
  // return;

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(slack_webhook_url);
    http.addHeader("Content-Type", "application/json");

    // JSON形式でペイロードを作成
    String payload = "{\"text\":\"" + message + "\"}";
    
    int httpResponseCode = http.POST(payload);
    Serial.print("Slack送信結果 (HTTPコード): ");
    Serial.println(httpResponseCode);
    http.end();
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  // for debug
  // Serial.print("【データ受信】サイズ: ");
  // Serial.println(length);

  String messageTemp;
  for (int i = 0; i < length; i++) {
    messageTemp += (char)payload[i];
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, messageTemp);

  if (error) {
    // JSONが大きすぎて解読できない場合はバッファ不足の可能性あり
    Serial.println("JSON解読エラー: データのサイズが大きすぎます");
    return;
  }

  // for debug：受信したデータの中に何が入っているか確認
  bool hasPrint = doc.containsKey("print");
  bool hasGcodeState = hasPrint && doc["print"].containsKey("gcode_state");
  bool hasRemainingTime = hasPrint && doc["print"].containsKey("mc_remaining_time");

  // 1. 状態(gcode_state)の更新
  if (hasGcodeState) {
    String newState = doc["print"]["gcode_state"].as<String>();
    if (newState != currentState && currentState != "UNKNOWN") {
      Serial.println("状態変化検知: " + currentState + " -> " + newState);
      
      if (newState == "FINISH") {
        sendSlackMessage("✅ プリントが正常に完了しました！");
        isStartNotified = false;
      } else if (newState == "PAUSE" || newState == "FAILED") {
        sendSlackMessage("⚠️ プリントが一時停止、またはエラーが発生しました。");
        isStartNotified = false;
      } else if (newState == "IDLE") {
        isStartNotified = false;
      }
    }
    currentState = newState;
  }

  // 2. 開始通知の処理
  if (currentState == "RUNNING" && !isStartNotified) {
    if (hasRemainingTime) {
      int remainingTime = doc["print"]["mc_remaining_time"].as<int>();
      
      if (remainingTime > 0) {
        // --- ここから「時間・分」への変換ロジック ---
        int hours = remainingTime / 60;
        int mins = remainingTime % 60;
        String durationStr = "";
        
        if (hours > 0) {
          durationStr = String(hours) + "時間" + String(mins) + "分";
        } else {
          durationStr = String(mins) + "分";
        }
        // ----------------------------------------

        String msg = "🚀 プリントを開始しました！\n合計推定時間: " + durationStr;
        
        // 完了予定時刻の計算（ここは以前のまま）
        time_t now;
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
          time(&now);
          now += (remainingTime * 60);
          struct tm *finishTime = localtime(&now);
          char timeStr[10];
          strftime(timeStr, sizeof(timeStr), "%H:%M", finishTime);
          msg += "\n完了予定時刻: " + String(timeStr);
        }

        sendSlackMessage(msg);
        isStartNotified = true;
        Serial.println("Slackへ通知を送信しました: " + durationStr);
      }
    }
  }
}

void setup_wifi() {
  delay(10);
  Serial.println("\nConnecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected! IP:");
  Serial.println(WiFi.localIP());

  Serial.flush(); 
}

void reconnect() {
  // MQTTに接続できるまでループ
  while (!client.connected()) {
    Serial.print("BambuプリンターのMQTTへ接続中...");
    
    // 任意のクライアントIDで接続
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("接続成功！");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("失敗しました。ステータス: ");
      Serial.print(client.state());
      Serial.println(" 5秒後に再試行します...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  setup_wifi();
  
  Serial.println("Step 1: Wi-Fi OK, waiting 1 sec...");
  Serial.flush();
  delay(1000);

  Serial.println("Step 2: NTP Time config...");
  Serial.flush();
  configTime(gmtOffset_sec, daylightOffset_sec, "ntp.nict.jp", "time.google.com");

  Serial.println("Step 3: MQTT setup...");
  Serial.flush();
  espClient.setInsecure(); 
  client.setServer(mqtt_server, 8883);
  client.setCallback(callback);
  client.setBufferSize(32768);

  Serial.println("Step 4: Setup complete!");
  Serial.flush();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // delay(50);
}