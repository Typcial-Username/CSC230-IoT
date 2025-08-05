#include <Arduino.h>
#include <PubSubClient.h>
#include <M5Core2.h>
#include <WiFi.h>

#include "secrets.h"
void connectToWiFi();
void mqttCallback(char *mqttTopic, byte *payload, unsigned int length);
void connectToMQTT();

WiFiClient espClient;
PubSubClient mqttClient(espClient);

int count = 0;
int lastMessagePos = 20;

void setup() {
  Serial.begin(115200);
  connectToWiFi();

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setKeepAlive(60);
  mqttClient.setCallback(mqttCallback);

  M5.begin();
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE);
}

void loop() {
  if (!mqttClient.connected()) {
    connectToMQTT();
  }
  mqttClient.loop();
}

void connectToWiFi() {
  // Implement WiFi connection logic here
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  Serial.println("Connected to WiFi");
}

void connectToMQTT() {
  while (!mqttClient.connected()) {
    String clientId = "esp32-client-" + String(WiFi.macAddress());
    Serial.printf("Connecting to MQTT Broker as %s.....\n", clientId.c_str());
    if (mqttClient.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("Connected to MQTT Broker");
      mqttClient.subscribe(MQTT_TOPIC);
      mqttClient.publish(MQTT_TOPIC, "Hello EMQX I'm a Core2!");
    } else {
      Serial.print("Failed to connect, state: ");
      Serial.println(mqttClient.state());
      delay(5000);
    }
  }
}

void mqttCallback(char *mqttTopic, byte *payload, unsigned int length) {
  Serial.print("Message received on topic: ");
  Serial.println(mqttTopic);
  Serial.print("Message: ");

  String message = "";

  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.println(message);

  Serial.println("\n--------------------");
  count++;

  M5.Lcd.setCursor(10, 0);
  M5.Lcd.print("Count: " + String(count));

  M5.Lcd.setCursor(10, lastMessagePos);
  M5.Lcd.print(message);
  lastMessagePos += 25;
}