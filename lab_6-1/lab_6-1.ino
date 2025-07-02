/*
* Lab 6.1: Using a Voltage and Analog Input to Determine Temperature
* Name: Levi Terry
* Email: lterry80052@uat.edu
* Due: 7/6/25
*/

#include <M5Core2.h>

#define THERM_PIN 35

void setup() {
  // Serial.begin(115200);
  // while (!Serial);
  // Serial.println("Serial Ready!");

  pinMode(THERM_PIN, INPUT);

  M5.begin();
  // Serial.println("Core2 Ready!");
  M5.Lcd.setTextSize(2);
  delay(1500);
}

// 12 bit (0 - 4095)
int adcVal = 0;
float voltVal = 0.0f; 
float thermVal = 0.0f;

float temp = 0.0f;

int adcMax = 4096;
float vcc  = 3.3f;

int R2 = 30000;

void loop() {
  adcVal = analogRead(THERM_PIN);

  M5.Lcd.setCursor(50, 50);
  M5.Lcd.print("ADC Val: ");
  M5.Lcd.println(adcVal);

  voltVal = (thermVal / adcMax) * vcc;

  M5.Lcd.setCursor(50, 100);
  M5.Lcd.print("Voltage: ");
  M5.Lcd.println(voltVal);

  thermVal = (vcc / (R2 * voltVal)) - R2;

  M5.Lcd.setCursor(50, 150);
  M5.Lcd.print("Thermister Val: ");
  M5.Lcd.println(thermVal);

  delay(100);
}
