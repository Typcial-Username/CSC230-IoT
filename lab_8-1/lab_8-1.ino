#include <IOXhop_FirebaseESP32.h>
#include <IOXhop_FirebaseStream.h>

#include <WiFi.h>
#include <M5Core2.h>

#include "secrets.h"

void setup() {
  M5.begin();
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Conntected: ");
  Serial.println(WiFi.localIP());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.setString("Screen_Color", "BLACK");
}

void loop() {
  String fireStatus = Firebase.getString("Screen_Color");

  M5.Lcd.fillScreen(fireStatus);
}
