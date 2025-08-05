/**
 * Lab 12.1: ESP32 and a soft Access Point
 * Author: Levi Terry
 * Email: lterry80052@uat.edu
 * Due Date:  8/3/25
 */

// Libraries
#include <Arduino.h>
#include <M5Core2.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>

const char *SSID = "M328P";
const char *PASSWORD = "arduino1";

WiFiServer server(80); // Create a server that listens on port 80

void setup() {
  Serial.begin(115200);
  Serial.println();
  
  Serial.println("Configuring Access Point...");
  if (!WiFi.softAP(SSID, PASSWORD)) {
    log_e("SoftAP creation failed.");
    while(1);
  }

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println("http://" + myIP.toString() + ":80");
  server.begin(); // Start the server
  Serial.println("Server started. Waiting for clients...");

  M5.begin();
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE);

  M5.Lcd.println("IP: ");
  M5.Lcd.println(WiFi.softAPIP());
}

void loop() {
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("Click <a href=\"/H\">here</a> to turn the screen BLUE.<br>");
            client.print("Click <a href=\"/L\">here</a> to turn the screen RED.<br>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          M5.Lcd.fillScreen(BLUE);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L")) {
          M5.Lcd.fillScreen(RED);                // GET /L turns the LED off
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}