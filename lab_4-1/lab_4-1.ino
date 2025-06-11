/*
* CSC230 - Lab 4.1 - IMU Sensors
* Author: Levi Terry
* Due Date: 6/15/25
*/
#include <M5Core2.h>

// Define variables for storing inertial sensor data
float accX = 0.0F;  
float accY = 0.0F;
float accZ = 0.0F;

// Maximum value for each axis
float maxX, maxY, maxZ = 0.0F;

float maxTotalVector = 0.0F;

bool isInFreeFall = false;
unsigned long freeFallStartTime = 0;

void setup() {
  M5.begin();
  M5.IMU.Init();
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setTextSize(2);
}

void loop() {
  M5.update();
  // Query the IMU for accleration
  M5.IMU.getAccelData(&accX, &accY, &accZ);

  // Check if any axis has a larger max
  if (accX > maxX) { maxX = accX; }
  if (accY > maxY) { maxY = accY; }
  if (accZ > maxZ) { maxZ = accZ; }

  // Print out the data
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.printf("accX,  accY, accZ");

  M5.Lcd.setCursor(0, 42);
  M5.Lcd.printf("%6.2f %6.2f%6.2f o/s", accX, accY, accZ);

  M5.Lcd.setCursor(0, 70);
  M5.Lcd.printf("maxX,  maxY, maxZ");

  M5.Lcd.setCursor(0, 92);
  M5.Lcd.printf("%6.2f %6.2f%6.2f o/s", maxX, maxY, maxZ);

  // Find the total vector of the max values
  maxTotalVector = sqrt((maxX * maxX) + (maxY * maxY) + (maxZ * maxZ));
  M5.Lcd.setCursor(0, 115);
  M5.Lcd.printf("maxTotalVector");
  M5.Lcd.setCursor(0, 135);
  M5.Lcd.printf("%6.2f", maxTotalVector);

  // Check the current accleration
  float totalAccel = sqrt((accX * accX) + (accY * accY) + (accZ * accZ));
  // There should be almost 0 acceleration during free fall
  if (totalAccel < 0.3)
  {
    if (!isInFreeFall)
    {
      isInFreeFall = true;
      freeFallStartTime = millis();
      // d = 1/2 * G * t^2 | 10 cm = 140ms falling time
    } else if (millis() - freeFallStartTime >= 140)
    {
      // If faling for 140ms (0.1 m) flash the screen red and white
      for (int i = 0; i < 5; i++)
      {
        M5.Lcd.fillScreen(RED);
        delay(250);
        M5.Lcd.fillScreen(WHITE);
        delay(250);
      }

      // Reset after triggering
      isInFreeFall = false;
      // Reset the screen
      M5.Lcd.fillScreen(BLACK);
    } 
  } else {
    isInFreeFall = false;
  }

  // Reset the max values if the left button was pressed
  if (M5.BtnA.wasReleased()) {
    maxX = 0;
    maxY = 0;
    maxZ = 0;
    maxTotalVector = 0;
  }
}
