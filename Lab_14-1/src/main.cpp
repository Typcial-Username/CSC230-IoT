/**
 * 14.1: Using APIs to obtain data
 * Fetches weather data from the OpenWeatherMap API and displays it on the M5Core2 screen.
 * Allows switching between two cities using buttons.
 * Author: Levi Terry
 * Email: lterry80053@uat.edu
 * Due: 8/17/25
 */

// Library imports
#include <Arduino.h>
#include <M5Core2.h>
#include <WiFi.h>
#include <Arduino_JSON.h>
#include <HTTPClient.h>

// Secret imports
#include "secrets.h"

// Global variables
String city = "Tempe";
String countryCode = "US";

JSONVar weatherData;

unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 10000; // Update every 10 seconds

String jsonBuffer;

void setup() {
  Serial.begin(115200);
  M5.begin();

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to WiFi...");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected to WiFi. IP Address: ");
  Serial.println(WiFi.localIP());

  M5.Lcd.setTextSize(2);
}

// Function to perform a HTTP GET request
String httpGETRequest(const char* uri)
{
  WiFiClient client;
  HTTPClient http;

  http.begin(client, uri);
  int httpCode = http.GET();

  String payload = "{}";

  if (httpCode > 0)
  {
    Serial.print("HTTP Response code: ");
    Serial.println(httpCode);

    payload = http.getString();
  } else {
    Serial.printf("HTTP GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();

  return payload;
}

void loop() {
  M5.update();

  // Check if it's time to update the weather data
  if ((millis() - lastUpdateTime) > updateInterval)
  {
    // Make sure the WiFi is connected before making the request
    if (WiFi.status() == WL_CONNECTED)
    {
      String apiURI = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&appid=" + OPEN_WEATHER_API_KEY + "&units=imperial";

      jsonBuffer = httpGETRequest(apiURI.c_str());
      Serial.println("Received JSON:");
      Serial.print("\t");
      Serial.println(jsonBuffer);

      weatherData = JSON.parse(jsonBuffer);

      // Check if parsing was successful
      if (JSON.typeof(weatherData) == "undefined") {
        Serial.println("Parsing JSON failed!");
        return;
      }
      
      // Display the data on the serial monitor
      Serial.println("Weather Data:");
      Serial.print("\tCity: ");
      Serial.println(weatherData["name"]);
      Serial.print("\tTemperature: ");
      Serial.print(weatherData["main"]["temp"]);
      Serial.println(" °F");
      Serial.print("\tPressure: ");
      Serial.print(weatherData["main"]["pressure"]);
      Serial.println(" hPa");
      Serial.print("\tHumidity: ");
      Serial.print(weatherData["main"]["humidity"]);
      Serial.println(" %");
      Serial.print("\tWind Speed: ");
      Serial.print(weatherData["wind"]["speed"]);
      Serial.println(" mph");

      Serial.print("\tWeather: ");
      Serial.println(weatherData["weather"][0]["description"]);
    } else {
      Serial.println("WiFi not connected. Cannot fetch weather data.");
    }

    // Update the last update time
    lastUpdateTime = millis();
  }

  // Switch between cities using buttons
  if (M5.BtnA.wasPressed() ) {
    city = "Tempe";
    countryCode = "US";
    M5.Lcd.fillRect(0, 20, 320, 30, BLACK);
  } else if (M5.BtnB.wasPressed()) {
    city = "Henderson";
    countryCode = "US";
    M5.Lcd.fillRect(0, 20, 320, 30, BLACK);
  }

  // Display the weather on the M5Core2 screen
  // Counntry and city
  M5.Lcd.setCursor(15, 10);
  M5.Lcd.print("Country: " + countryCode); 
  M5.Lcd.setCursor(15, 30);
  M5.Lcd.print("City: " + city);

  if (JSON.typeof(weatherData) != "undefined") {
    // Clear if last request was unsuccessful
    M5.Lcd.fillRect(0, 45, 320, 20, BLACK);

    // Display weather data
    M5.Lcd.setCursor(15, 70);
    M5.Lcd.print("Temperature: ");
    M5.Lcd.print(weatherData["main"]["temp"]);
    M5.Lcd.print(" *F");

    M5.Lcd.setCursor(15, 90);
    M5.Lcd.print("Pressure: ");
    M5.Lcd.print(weatherData["main"]["pressure"]);

    M5.Lcd.print(" hPa");
    M5.Lcd.setCursor(15, 110);

    M5.Lcd.print("Humidity: ");
    M5.Lcd.print(weatherData["main"]["humidity"]);
    M5.Lcd.print(" %");
    M5.Lcd.setCursor(15, 130);
    M5.Lcd.print("Wind Speed: ");

    M5.Lcd.print(weatherData["wind"]["speed"]);
    M5.Lcd.print(" mph");
    M5.Lcd.setCursor(15, 150);

    M5.Lcd.print("Weather: ");
    M5.Lcd.print(weatherData["weather"][0]["description"]);
  } else {
    M5.Lcd.setCursor(15, 50);
    M5.Lcd.print("No data available");
  }

}
