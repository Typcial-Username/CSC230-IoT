/**
 * CSC230 Final
 * Author: Levi Terry
 * 
 * Description: Fetches random quotes from the Quotable API and displays them on the screen. Has autofetch functionality and shows the current time
 */

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <M5Core2.h>
#include <Arduino_JSON.h>
#include <algorithm>

#include "secrets.h"
#include "time.h"

// Prototypes
String getQuote();
JSONVar parseJson(String jsonString);
void drawWrappedText(String text, String author, int x, int y, int textSize = 2);
String formatAutofetchDelay(int timeout);
String getCurrentTime();
String convertTo12HTime(String time24);

int autoFetchTimeY = 25;
int quoteY = 60;

// Autofetch settings
bool autoFetch = false;
int autoFetchTimeout = 30000;
unsigned long lastFetchTime = 0;

// Timeout options 5s, 10s, 20s, 30s, 1m, 30m, 1h
int timeouts[] = {5000, 10000, 20000, 30000, 60000, 1800000, 3600000};

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -7 * 3600;
const int daylightOffset_sec = 0;

unsigned long lastTimeUpdate = 0;
static int lastMinute = -1;
struct tm startTimeInfo;

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

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  Serial.println("Current time: " + convertTo12HTime(getCurrentTime()));

  M5.begin();

  M5.Lcd.fillScreen(GREEN);
  M5.Lcd.setTextSize(2);

  JSONVar quote = parseJson(getQuote());
  M5.Lcd.setTextColor(BLACK, GREEN);
  drawWrappedText(quote["content"], quote["author"], 5, quoteY);
}

void loop() {
  M5.update();

  // Update time on screen (1/min)
  if (getLocalTime(&startTimeInfo)) {
    if (startTimeInfo.tm_min != lastMinute) {
      lastMinute = startTimeInfo.tm_min;
      String time = convertTo12HTime(getCurrentTime());

      Serial.println("Current time: " + time);

      // Center
      int x = (M5.Lcd.width() - M5.Lcd.textWidth(time)) / 2;
      int y = 5;

      M5.Lcd.setCursor(x - 10, y);

      M5.Lcd.fillRect(x - 10, y, M5.Lcd.textWidth("00:00"), 16, GREEN);

      M5.Lcd.setTextColor(DARKGREY, GREEN);
      M5.Lcd.drawString(time, x - 10, y);
    }
  }

  // BtnA manually gets quote
  if (M5.BtnA.wasPressed()) {
    Serial.println("Fetching new quote...");

    M5.Lcd.setCursor(M5.Lcd.width() - M5.Lcd.textWidth("Loading...") - 10, M5.Lcd.height() - 30);
    M5.Lcd.println("Loading...");

    JSONVar quote = parseJson(getQuote());

    // M5.Lcd.setCursor(10, 30);
    M5.Lcd.fillRect(0, quoteY, M5.Lcd.width(), M5.Lcd.height() - 20, GREEN);
    M5.Lcd.setTextColor(BLACK, GREEN);
    drawWrappedText(quote["content"], quote["author"], 5, quoteY);

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

    M5.Lcd.fillRect(0, autoFetchTimeY, M5.Lcd.width(), 20, GREEN);
    
    Serial.println("Changed auto-fetch timeout to: " + String(autoFetchTimeout) + "ms");
  }

  if (autoFetch) {
    // Display current autofetch delay
    M5.Lcd.fillCircle(M5.Lcd.width() / 2, M5.Lcd.height() - 10, 5, BLUE);
    M5.Lcd.setCursor(10, autoFetchTimeY);
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
  
      M5.Lcd.fillRect(0, quoteY, M5.Lcd.width(), M5.Lcd.height() - quoteY, GREEN);
      M5.Lcd.setTextColor(BLACK, GREEN);
      drawWrappedText(quote["content"], quote["author"], 5, quoteY);
    }
  } else {
    M5.Lcd.fillRect((M5.Lcd.width() / 2) - 5, M5.Lcd.height() - 10, 10, 10, GREEN);
    M5.Lcd.setCursor(10, autoFetchTimeY);
    M5.Lcd.fillRect(10, autoFetchTimeY, M5.Lcd.width(), quoteY - autoFetchTimeY, GREEN);
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

String getCurrentTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return String("");
  }

  // Pad hour, min, sec with leading zeros
  String hh = String(timeinfo.tm_hour);   // keep hour as-is for 24H
  String mm = timeinfo.tm_min < 10 ? "0" + String(timeinfo.tm_min) : String(timeinfo.tm_min);
  // String ss = timeinfo.tm_sec < 10 ? "0" + String(timeinfo.tm_sec) : String(timeinfo.tm_sec);

  return hh + ":" + mm/* + ":" + ss*/;
}

String convertTo12HTime(String time24) {
  Serial.println("Converting 24H time: " + time24);
  int colonIndex = time24.indexOf(':');
  if (colonIndex == -1) {
    return time24; // Invalid format, return as is
  }

  int hour = time24.substring(0, colonIndex).toInt();
  String minutes = time24.substring(colonIndex + 1);
  String period = "AM";

  if (hour >= 12) {
    period = "PM";

    if (hour > 12) {
      hour -= 12;
    }
  } else if (hour == 0) {
    hour = 12; // Midnight case
  }

  return String(hour) + ":" + minutes + " " + period;
}