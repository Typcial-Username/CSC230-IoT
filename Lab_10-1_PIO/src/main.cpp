/**
 * Assignment 10.1: Using Google Firebase to Record Data
 * Author: Levi Terry
 * Due: 7/20/25
 *
 * Description: Uses the FirebaseClient library to recieve an int and a float from Firebase and display the results on an M5Stack Core2.
 */

 // Imports
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <M5Core2.h>

// Firebaase
#include <FirebaseClient.h>
#include <FirebaseJson.h>

// Credentials
#include "secrets.h"

// Global variables
using AsyncClient = AsyncClientClass;

WiFiClientSecure sslClient;

AsyncClient aClient(sslClient);
UserAuth userAuth(FIREBASE_AUTH, FIREBASE_EMAIL, FIREBASE_PASSWORD);

FirebaseApp app;

FirebaseClient firebaseClient;
RealtimeDatabase database;

// AsyncResult databaseResult;
AsyncResult intResult;
AsyncResult floatResult;

unsigned long recieveDataPrevMillis = 0;

// How long to wait between data requests
#define UPDATE_INTERVAL 30000

// Prototypes
void processData(AsyncResult &aResult);
void getData(String path);

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  Serial.print("Connected to WiFi with IP: ");
  Serial.println(WiFi.localIP());

  // Initialize Firebase 
  sslClient.setInsecure();
  
  initializeApp(aClient, app, getAuth(userAuth), processData);
  app.getApp<RealtimeDatabase>(database);
  database.url(FIREBASE_HOST);

  // Initialize the Core2
  M5.begin();

  M5.Lcd.fillScreen(BLUE);
  M5.Lcd.setTextColor(WHITE, BLUE);
  M5.Lcd.setTextSize(2);
}

unsigned long authStart = millis();

void loop() {
  app.loop();

  // for (int i = 0; i < 10; i++) {
  //   app.loop();
  //   delay(5); // Short delay to let other processes run
  // }

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

  // Make sure not to fetch data too frequently
  if (millis() - recieveDataPrevMillis < UPDATE_INTERVAL) return;
  recieveDataPrevMillis = millis();

  // Where the data is
  String intPath = "test/int";
  String floatPath = "test/float";

  // Get the data
  getData(intPath);
  getData(floatPath);

  // Check for errors
  if (aClient.lastError().code() == 0)
  {
    Serial.println("Operation Successful");
  } else 
  {
    Serial.print("Error getting value: ");
    Serial.print(aClient.lastError().code());
    Serial.println(aClient.lastError().message());
  }
}

// Callback function for incoming data (needed, but unused)
void processData(AsyncResult &aResult)
{
  Serial.println("Received data");
  if (!aResult.isResult()) {
    Serial.println("Not a result");
  };

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

void getData(String path) {
  Serial.printf("Getting data from path: %s\n", path.c_str());

  if (path.equals("test/int")) {
    // Get the int
    database.get(aClient, path, intResult);
    
    // Wait for the data to be avaliable
    if (intResult.available()) {
      Serial.printf("Integer value: %d\n", String(intResult.c_str()).toInt());

      // Display the value on the screen
      M5.Lcd.setCursor(10, 30);
      M5.Lcd.printf("test/int: %d", String(intResult.c_str()).toInt());
    } else {
      Serial.println("No integer data available");
    }
  } else if (path.equals("test/float")) {
    // Get the float
    database.get(aClient, path, floatResult);

    // Wait for the data to be avaliable
    if (floatResult.available()) {
      Serial.printf("Float value: %f\n", String(floatResult.c_str()).toFloat());

      // Display the value on the screen
      M5.Lcd.setCursor(10, 50);
      M5.Lcd.printf("test/float: %f", String(floatResult.c_str()).toFloat());
    } else {
      Serial.println("No float data available");
    }
  }

  // Check for immediate errors
  if (aClient.lastError().code() != 0) {
    Serial.printf("Error on request: %d - %s\n", 
      aClient.lastError().code(), 
      aClient.lastError().message().c_str());
  }
}