/**
 * Assignment 9.1: Using Google Firebase to Record Data
 * Author: Levi Terry
 * Due: 7/20/25
 *
 * Description: Uses the FirebaseClient library to send an int and a float to Firebase and display the results on an M5Stack Core2.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <M5Core2.h>

#include <FirebaseClient.h>
#include <FirebaseJson.h>

#include "secrets.h"

// Global variables
using AsyncClient = AsyncClientClass;

WiFiClientSecure sslClient;

AsyncClient aClient(sslClient);
UserAuth userAuth(FIREBASE_AUTH, FIREBASE_EMAIL, FIREBASE_PASSWORD);

FirebaseApp app;

FirebaseClient firebaseClient;
RealtimeDatabase database;

AsyncResult databaseResult;

unsigned long recieveDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

// Prototypes
void processData(AsyncResult &aResult);
void getData(String path);

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  Serial.print("Connected to WiFi with IP: ");
  Serial.println(WiFi.localIP());

  sslClient.setInsecure();
  initializeApp(aClient, app, getAuth(userAuth), processData);
  app.getApp<RealtimeDatabase>(database);
  database.url(FIREBASE_HOST);

  M5.begin();

  M5.Lcd.fillScreen(BLUE);
  M5.Lcd.setTextColor(WHITE, BLUE);
  M5.Lcd.setTextSize(2);
}

unsigned long authStart = millis();

void loop() {
  app.loop();

  if (!app.ready())
  {
    if (millis() - authStart > 10000)
    {
      Serial.println("Firebase App not ready after 10 seconds.");
      authStart = millis(); // Reset the timer
    } else {
      Serial.print("Waiting for auth... elapsed: ");
      Serial.print((millis() - authStart) / 1000);
      Serial.println(" s");
    }

    delay(1000);
    return;
  }

  if (millis() - recieveDataPrevMillis < 5000) return;
  recieveDataPrevMillis = millis();

  String intPath = "test/int";
  database.set(aClient, intPath, count);

  Serial.printf("Set %s to %d\n", intPath.c_str(), count);
  M5.Lcd.setCursor(10, 30);
  M5.Lcd.printf("test/int: %d", count);

  // Increment count
  count++;

  // Write a float on the database path test/float
  float num = 0.01 + random(0, 100);
  String floatPath = "test/float";
  database.set(aClient, floatPath, num);

  Serial.printf("Set %s to %f\n", floatPath.c_str(), num);
  M5.Lcd.setCursor(10, 50);
  M5.Lcd.printf("test/float: %f", num);

  if (aClient.lastError().code() == 0)
  {
    Serial.println("Operation Successful");
  } else 
  {
    Serial.print("Error setting value: ");
    Serial.print(aClient.lastError().code());
    Serial.println(aClient.lastError().message());
  }
}

void processData(AsyncResult &aResult)
{
  if (!aResult.isResult()) return;

  if (aResult.isEvent()) Serial.printf("Event received\n\t UID: %s\nMessage: %s\nCode: %d\n", aResult.uid().c_str(), aResult.eventLog().message().c_str(), aResult.eventLog().code());

  if (aResult.isDebug())
    Serial.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());

  if (aResult.isError())
    Serial.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());

  if (aResult.available()) 
  {
    Serial.printf("Firebase data received\n\t UID: %s\nPayload: %s\n", aResult.uid().c_str(), aResult.payload().c_str());
  }
}
