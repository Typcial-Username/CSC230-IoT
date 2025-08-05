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
#include <SPIFFS.h>

#include "secrets.h"

WiFiServer server(80); // Create a server that listens on port 80

// Prototype for the function to serve files
void serveFile(WiFiClient client, String filename, String contentType);
void listSPIFFSFiles();

void setup() {
  Serial.begin(115200);
  Serial.println();

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  
  Serial.println("Configuring Access Point...");
  if (!WiFi.softAP(SSID, PASSWORD)) {
    log_e("SoftAP creation failed.");
    while(1);
  }

  // Configure AP
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println("http://" + myIP.toString() + ":80");
  server.begin(); // Start the server
  Serial.println("Server started. Waiting for clients...");

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  M5.begin();
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE);

  M5.Lcd.println("IP: ");
  M5.Lcd.println(WiFi.softAPIP());
  M5.Lcd.println("No Flag Selected");
}

void loop() {
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";      
    String header = "";
    // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();   
        header += c;          // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // Check if requesting an image file
            if (header.indexOf("GET /be-flag.gif") >= 0)
            {
              serveFile(client, "/be-flag.gif", "image/gif");
              break;
            } 
            else if (header.indexOf("GET /mn-flag.gif") >= 0)
            {
              serveFile(client, "/mn-flag.gif", "image/gif");
              break;
            } else {
              // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
              // and a content-type so the client knows what's coming, then a blank line:
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println();
  
              // the content of the HTTP response follows the header:
              client.print("<!DOCTYPE html><html><body style=\"background-color:#ccc;\">");
              client.print("<h1 style=\"text-align:center;\">Flag Selector</h1>");
              client.print("<div style=\"display:flex;flex-direction:row;align-content:center;justify-content:center;gap:1.25rem;\">");
              client.print("<a href=\"B\"><span style=\"display:flex;flex-direction:column;\">Belgum <img src=\"/be-flag.gif\" /></span></a>");
              client.print("<a href=\"M\"><span style=\"display:flex;flex-direction:column;\">Monaco <img src=\"/mn-flag.gif\" /></span></a>");
              client.print("</div>");
              client.print("<p id=\"status\" style=\"text-align:center;\"></p>");
              client.print("<script>");
              client.print("const status = document.getElementById(\"status\");");
              client.print("if (document.location.pathname == \"/B\") { status.innerText = \"Set Core2 BG to Belgum\"; } else if (document.location.pathname == \"/M\") { status.innerText = \"Set Core2 BG to Monaco\"; }");
              client.print("</script>");
              client.print("</body></html>");

              // The HTTP response ends with another blank line:
              client.println();
              // break out of the while loop:
              break;
            }

          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /B")) {
          M5.Lcd.fillScreen(BLACK);

          // Draw Belgum
          M5.Lcd.fillRect(100, 0, 100, 240, YELLOW);
          M5.Lcd.fillRect(200, 0, 100, 240, RED);

        }
        if (currentLine.endsWith("GET /M")) {
          M5.Lcd.fillScreen(BLACK);       
          // Draw Monaco
          M5.Lcd.fillRect(0, 0, 320, 120, RED);
          M5.Lcd.fillRect(0, 120, 320, 120, WHITE);         
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}

void serveFile(WiFiClient client, String filename, String contentType)
{
  Serial.println("Serving file: " + filename);
  // Check if the file exists in SPIFFS
  if (SPIFFS.exists(filename)) {
    File file = SPIFFS.open(filename, "r");

    if (!file) {
      Serial.println("Failed to open file for reading");
      client.println("HTTP/1.1 500 Internal Server Error");
      client.println();
      return;
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: " + contentType);
    client.println("Content-Length: " + String(file.size()));
    client.println();

    // Send file in chunks
    uint8_t buffer[1024];
    size_t totalBytes = 0;
    
    while (file.available()) {
      size_t bytesRead = file.readBytes((char *)buffer, sizeof(buffer));
      client.write(buffer, bytesRead);
      totalBytes += bytesRead;
    }

    file.close();
    Serial.println("Sent " + String(totalBytes) + " bytes");
  } else {
    Serial.println("File NOT found: " + filename);
    client.println("HTTP/1.1 404 Not Found");
    client.println("Content-Type: text/plain");
    client.println();
    client.println("File not found: " + filename);
  }
}

void listSPIFFSFiles() {
  Serial.println("Listing SPIFFS files:");
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while (file) {
    Serial.print("\tFile: ");
    Serial.print(file.name());
    Serial.print(" Size: ");
    Serial.println(file.size());
    file = root.openNextFile();
  }
}