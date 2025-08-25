/**
 * CSC230 Final
 * Author: Levi Terry
 * 
 * Description: Fetches random quotes from the Quotable API and displays them on the screen. Has autofetch
 */

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <M5Core2.h>
#include <Arduino_JSON.h>
#include <algorithm>

#include "secrets.h"

// Prototypes
String getQuote();
JSONVar parseJson(String jsonString);
void drawWrappedText(String text, String author, int x, int y, int textSize = 2);
String formatAutofetchDelay(int timeout);

// Autofetch settings
bool autoFetch = false;
int autoFetchTimeout = 30000;
unsigned long lastFetchTime = 0;

// Timeout options 5s, 10s, 20s, 30s, 1m, 30m, 1h
int timeouts[] = {5000, 10000, 20000, 30000, 60000, 1800000, 3600000};

void setup() {
  Serial.begin(115200);
  // while (!Serial) {;}

  // Connect to WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();

  IPAddress localIP = WiFi.localIP();
  Serial.print("Connected to WiFi. IP Address: ");
  Serial.println(localIP);
  Serial.println();

  M5.begin();

  M5.Lcd.fillScreen(GREEN);
  M5.Lcd.setTextSize(2);
}

void loop() {
  M5.update();

  // BtnA manually gets quote
  if (M5.BtnA.wasPressed()) {
    Serial.println("Fetching new quote...");

    M5.Lcd.setCursor(M5.Lcd.width() - M5.Lcd.textWidth("Loading...") - 10, M5.Lcd.height() - 30);
    M5.Lcd.println("Loading...");

    JSONVar quote = parseJson(getQuote());

    // M5.Lcd.setCursor(10, 30);
    M5.Lcd.fillScreen(GREEN);
    M5.Lcd.setTextColor(BLACK, GREEN);
    drawWrappedText(quote["content"], quote["author"], 5, 30);

    // BtnB toggles auto-fetch
  } else if (M5.BtnB.pressedFor(500, 1500)) {
    autoFetch = !autoFetch;
    // BtnC changes auto-fetch timeout
  } else if (M5.BtnC.pressedFor(300, 1000)) {
    int timeoutsLen = sizeof(timeouts)/sizeof(timeouts[0]);
    
    auto it = std::find(timeouts, timeouts + timeoutsLen, autoFetchTimeout);
    int currentTimeoutIdx = (it != timeouts + timeoutsLen) ? (it - timeouts) : 0;
    
    // Move to next timeout in the array
    currentTimeoutIdx = (currentTimeoutIdx + 1) % timeoutsLen;
    autoFetchTimeout = timeouts[currentTimeoutIdx];

    M5.Lcd.fillRect(0, 0, M5.Lcd.width(), 20, GREEN);
    
    Serial.println("Changed auto-fetch timeout to: " + String(autoFetchTimeout) + "ms");
  }

  if (autoFetch) {
    // Display current autofetch delay
    M5.Lcd.fillCircle(320 / 2, 230, 5, BLUE);
    M5.Lcd.setCursor(10, 5);
    M5.Lcd.setTextColor(BLUE, WHITE);
    M5.Lcd.println("Autofetch delay: " + formatAutofetchDelay(autoFetchTimeout));
    M5.Lcd.setTextColor(BLACK, GREEN);

    // Check if another fetch is needed
    if ((millis() - lastFetchTime > autoFetchTimeout)) {
      lastFetchTime = millis();
      Serial.println("Auto-fetching new quote...");

      M5.Lcd.setCursor(M5.Lcd.width() - M5.Lcd.textWidth("Loading...") - 10, M5.Lcd.height() - 30);
      M5.Lcd.println("Loading...");

      JSONVar quote = parseJson(getQuote());
  
      M5.Lcd.fillScreen(GREEN);
      M5.Lcd.setTextColor(BLACK, GREEN);
      drawWrappedText(quote["content"], quote["author"], 5, 30);
    }
  } else {
    M5.Lcd.fillRect((320 / 2) - 5, 225, 10, 10, GREEN);
    M5.Lcd.setCursor(10, 0);
    M5.Lcd.fillRect(10, 0, 300, 20, GREEN);
  }
}

// Make the request to the Quotable API and return the response as a string
String getQuote() {
  WiFiClientSecure client;
  client.setInsecure(); // Disable certificate verification for simplicity
  
  HTTPClient http;

  const char* host = "https://api.quotable.io/random";
  http.begin(client, host);

  int httpResponseCode = http.GET();
  String response;

  if (httpResponseCode > 0) {
    response = http.getString();
  } else {
    Serial.print("Error on HTTP request: ");
    Serial.println(httpResponseCode);
  }

  http.end();

  return response;
}

// Parse the JSON string and return a JSONVar object
JSONVar parseJson(String jsonString) {
  JSONVar json = JSON.parse(jsonString);

  if (JSON.typeof(json) == "undefined") {
    Serial.println("Parsing JSON failed!");
    return JSONVar();
  }

  return json;
}

// Draw text on the screen, wrapping it if necessary
void drawWrappedText(String text, String author, int x, int y, int textSize) {
  M5.Lcd.setTextSize(textSize);
  M5.Lcd.setCursor(x, y);

  int maxWidth = M5.Lcd.width() - 20; // Leave some margin

  // Approximation of line height
  int lineHeight = 8 * textSize;
  // How many lines were drawn
  int linesDrawn = 0;

  // Keep track of the start position of the current line
  int start = 0;
  String line = "";

  // Loop through the text and wrap if necessary
  while (start < text.length()) {
    /// Find when the next word is
    int spaceIdx =  text.indexOf(' ', start);

    if (spaceIdx == -1) {
      spaceIdx = text.length();
    }

    String word = text.substring(start, spaceIdx + 1);
    String testLine = line + word;

    // If the line will exceed the max width, finalize the current line
    if (M5.Lcd.textWidth(testLine) > maxWidth) {
      // If the test line exceeds the max width, we need to finalize the current line
      M5.Lcd.setCursor(x, y + linesDrawn * lineHeight);
      M5.Lcd.println(line);
      line = word;
      linesDrawn++;
    } else {
      line = testLine;
    }

    start = spaceIdx + 1;
  }
      
  if (line.length() > 0) {
    M5.Lcd.println(line);
    linesDrawn++;
  }

  // Find where to place the author name, right aligned
  int nextY = y + linesDrawn * lineHeight + 20;
  int authorWidth = (author.length() + 2) * 6 * textSize;
  int authorX = M5.Lcd.width() - authorWidth - 10;

  M5.Lcd.setCursor(authorX, nextY);
  // M5.Lcd.setTextDatum
  M5.Lcd.print("- " + author);
}

// Format the autofetch delay (adds correct unit)
String formatAutofetchDelay(int timeout) {
  if (timeout >= 3600000) {
    return String(timeout / 3600000) + "h";
  } else if (timeout >= 60000) {
    return String(timeout / 60000) + "m";
  } else  {
    return String(timeout / 1000) + "s";
  }
}
