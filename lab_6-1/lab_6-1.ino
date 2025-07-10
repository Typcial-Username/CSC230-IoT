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

float temp, tempC, tempF = 0.0;

float adcMax = 4095;
float vcc  = 3.3;

float R1 = 30000.0;
float R2 = 0.0f;

float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;

float logR2 = 0.0;

void loop() {
  adcVal = analogRead(THERM_PIN);

  M5.Lcd.setCursor(10, 10);
  M5.Lcd.print("ADC Val: ");
  M5.Lcd.println(adcVal);

  R2 = R1 * (adcMax / adcVal - 1);

  M5.Lcd.setCursor(10, 50);
  M5.Lcd.print("R2: ");
  M5.lcd.println(R2);

  voltVal = (adcVal / adcMax) * vcc;

  M5.Lcd.setCursor(10, 90);
  M5.Lcd.print("Voltage: ");
  M5.Lcd.println(voltVal);

  thermVal = (vcc / (R2 * voltVal)) - R1;

  M5.Lcd.setCursor(10, 130);
  M5.Lcd.print("Thermister Val: ");
  M5.Lcd.println(thermVal);

  logR2 = log(R2);
  temp = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));

  tempC = temp - 273.15;
  tempF = (tempC * 9.0) / 5.0 + 32;

  M5.Lcd.setCursor(10, 170);
  M5.Lcd.print("Temp: ");
  M5.Lcd.print(tempF);
  M5.lcd.print("*F (");
  M5.Lcd.print(tempC);
  M5.Lcd.println("*C)");

  delay(100);
}
