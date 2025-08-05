/**
* Assignment 9.1: Using Google Firebase to Record Data
* Author: Levi Terry
* Email: lterry80052@uat.edu
* Due: 7/20/25
*/

#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <M5Core2.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#include "secrets.h"

// Information for Firebase
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

void setup() {
  Serial.begin(115200);
  
  // Connect to WiFi
  WiFi.begin(WIFI_SSID< WIFI_PASSWORD);
  Serial.print("Connecting to WiFI");
  while (WiFi.status() != WL_CONNECTED
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = FIREBASE_AUTH;
  config.database_url = FIREBASE_HOST;

  // Connect to Firebase
  if (Firebase.signup(&config, &auth, "", ""))
  {
    Serial.println("Connected to 🔥 Firebase");
    signupOK = true;
  } else 
  {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Initialize the Core2
  M5.begin();
}

void loop() {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDatMillis == 0))
  {
    sendDataPrevMillis();

    // Write an int on the database path test/int
    if (Firebase.RTDB.setInt(&fbdo, "test/int", count)) {
      Serial.println("COUNT PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    } else {
      Serial.println("❌ COUNT FAILED");
      Serial.println("REASON " + fbdo.errorReason());
    }

    // Increment count
    count++;

    // Write a float on the database path test/float
    if (Firebase.RTDB.setInt(&fbdo, "test/float", 0.01 + random(0, 100))) {
      Serial.println("FLAOT PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    } else {
      Serial.println("❌ FLOAT FAILED");
      Serial.println("REASON " + fbdo.errorReason());
    }
  }
}
