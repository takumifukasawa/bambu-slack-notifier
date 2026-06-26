#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <time.h>
#include "secrets.h"

// ==========================================
// Config lives in secrets.h (copy secrets.example.h to create it)
// ==========================================
const char* ssid = WIFI_SSID;          // Wi-Fi SSID
const char* password = WIFI_PASSWORD;  // Wi-Fi password

const char* mqtt_server = MQTT_SERVER;      // Printer's local IP address
const char* mqtt_user = "bblp";               // Fixed as "bblp" on Bambu
const char* mqtt_password = MQTT_PASSWORD; // LAN Access Code from the printer screen
const char* mqtt_topic = MQTT_TOPIC; // device/<serial-number>/report

const char* slack_webhook_url = SLACK_WEBHOOK; // Slack Webhook URL
// ==========================================

// ==========================================
// Slack notification messages (edit freely / translate here)
// Static messages are plain constants; the dynamic "start" message is
// assembled in buildStartMessage() / formatDuration() below.
// ==========================================
const char* MSG_FINISH = "✅ プリントが正常に完了しました！";
const char* MSG_PAUSE  = "⚠️ プリントが一時停止、またはエラーが発生しました。";
// ==========================================

WiFiClientSecure espClient;
PubSubClient client(espClient);

String currentState = "UNKNOWN";
bool isStartNotified = false;

const long gmtOffset_sec = 32400; // JST (9 hours * 3600 sec)
const int daylightOffset_sec = 0;

// Send a message to Slack
void sendSlackMessage(String message) {
  // for debug
  // Serial.println("Slack notification preview:");
  // Serial.println(message);
  // return;

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(slack_webhook_url);
    http.addHeader("Content-Type", "application/json");

    // Build the JSON payload
    String payload = "{\"text\":\"" + message + "\"}";

    int httpResponseCode = http.POST(payload);
    Serial.print("Slack POST result (HTTP code): ");
    Serial.println(httpResponseCode);
    http.end();
  }
}

// Format a duration in minutes, e.g. "2時間30分" / "30分".
// To translate, rewrite the returned strings (word order is free here).
String formatDuration(int totalMinutes) {
  int hours = totalMinutes / 60;
  int mins = totalMinutes % 60;
  if (hours > 0) {
    return String(hours) + "時間" + String(mins) + "分";
  }
  return String(mins) + "分";
}

// Build the whole "print started" message.
// finishTime is the expected finish time (e.g. "14:30"); empty to omit it.
// To translate, edit this function body in one place.
String buildStartMessage(int remainingMinutes, const String& finishTime) {
  String msg = "🚀 プリントを開始しました！\n合計推定時間: " + formatDuration(remainingMinutes);
  if (finishTime.length() > 0) {
    msg += "\n完了予定時刻: " + finishTime;
  }
  return msg;
}

void callback(char* topic, byte* payload, unsigned int length) {
  // for debug
  // Serial.print("[received] size: ");
  // Serial.println(length);

  String messageTemp;
  for (int i = 0; i < length; i++) {
    messageTemp += (char)payload[i];
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, messageTemp);

  if (error) {
    // If the JSON is too large to parse, the buffer may be too small
    Serial.println("JSON parse error: payload size is too large");
    return;
  }

  // Check what the received payload contains
  bool hasPrint = doc.containsKey("print");
  bool hasGcodeState = hasPrint && doc["print"].containsKey("gcode_state");
  bool hasRemainingTime = hasPrint && doc["print"].containsKey("mc_remaining_time");

  // 1. Update the state (gcode_state)
  if (hasGcodeState) {
    String newState = doc["print"]["gcode_state"].as<String>();
    if (newState != currentState && currentState != "UNKNOWN") {
      Serial.println("State change detected: " + currentState + " -> " + newState);

      if (newState == "FINISH") {
        sendSlackMessage(MSG_FINISH);
        isStartNotified = false;
      } else if (newState == "PAUSE" || newState == "FAILED") {
        sendSlackMessage(MSG_PAUSE);
        isStartNotified = false;
      } else if (newState == "IDLE") {
        isStartNotified = false;
      }
    }
    currentState = newState;
  }

  // 2. Handle the start notification
  if (currentState == "RUNNING" && !isStartNotified) {
    if (hasRemainingTime) {
      int remainingTime = doc["print"]["mc_remaining_time"].as<int>();

      if (remainingTime > 0) {
        // Compute the expected finish time (empty if NTP not ready yet)
        String finishTime = "";
        time_t now;
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
          time(&now);
          now += (remainingTime * 60);
          struct tm *finish = localtime(&now);
          char timeStr[10];
          strftime(timeStr, sizeof(timeStr), "%H:%M", finish);
          finishTime = String(timeStr);
        }

        sendSlackMessage(buildStartMessage(remainingTime, finishTime));
        isStartNotified = true;
        Serial.println("Sent notification to Slack: " + formatDuration(remainingTime));
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
  // Loop until connected to MQTT
  while (!client.connected()) {
    Serial.print("Connecting to the Bambu printer's MQTT...");

    // Connect with an arbitrary client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected!");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("failed, state: ");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds...");
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
