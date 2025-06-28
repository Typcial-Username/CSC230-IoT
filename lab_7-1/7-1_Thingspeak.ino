/*
  Lab 7.1 ThingSpeak
  Simple mood tracker using M5Core2 and ThingSpeak

  Name: Levi Terry
  Email: lterry80052uat.edu
  Due Date: 6/28/25
  Class: CSC230: Internet of Things
  Professor: Prof. Prater
*/

#include <WiFi.h>
#include <M5Core2.h>
#include "secrets.h"
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros

char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

// -1 = No mood, 0 = Happy, 1 = Neutral/Content, 2 = Sad
int mood = -1;

unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 20000;

void setup() {
  Serial.begin(115200);  //Initialize serial
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo native USB port only
  }
  
  M5.begin();
  WiFi.mode(WIFI_STA);   
  ThingSpeak.begin(client);  // Initialize ThingSpeak

  M5.lcd.setTextSize(2);
}

void loop() {
  M5.update();

  M5.Lcd.setCursor(50, 0);
  M5.lcd.println("What's your mood?");

  M5.Lcd.setCursor(40, 200);
  M5.Lcd.println("H");
  M5.Lcd.setCursor(150, 200);
  M5.Lcd.println("N");
  M5.Lcd.setCursor(260, 200);
  M5.Lcd.println("S");

  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }

  if (M5.BtnA.wasReleased())
  {
    mood = 0;
    M5.Lcd.fillScreen(GREEN);
    M5.Lcd.setTextColor(WHITE, GREEN);
  } else if (M5.BtnB.wasReleased())
  {
    mood = 1;
    M5.Lcd.fillScreen(YELLOW);
    M5.Lcd.setTextColor(WHITE, YELLOW);
  } else if (M5.BtnC.wasReleased())
  {
    mood = 2;
    M5.Lcd.fillScreen(BLUE);
    M5.Lcd.setTextColor(WHITE, BLUE);
  }

  if (millis() - lastUpdateTime >= updateInterval && mood != -1)
  {
    int x = ThingSpeak.writeField(myChannelNumber, 1, mood, myWriteAPIKey);
    if(x == 200){
      Serial.print("Channel update successful.");
      Serial.print("The mood was: ");
      Serial.println(mood);
    }
    else{
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }

    lastUpdateTime = millis();
  }

  // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
  // pieces of information in a channel.  Here, we write to field 1.
  // 

  // M5.Lcd.fillScreen(BLACK);
  // M5.Lcd.print(number);
  
  // // change the value
  // number++;
  // if(number > 99){
  //   number = 0;
  // }
  
  // delay(20000); // Wait 20 seconds to update the channel again
}
