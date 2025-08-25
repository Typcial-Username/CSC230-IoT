#include <M5Core2.h>

#define THER_PIN  35;

void setup() {
  while (!Serial);
  Serial.begin(115200);  
  M5.begin();

}

int therVal;

void loop() {
  // put your main code here, to run repeatedly:
  therVal = analogRead(THER_PIN);
  Serial.println(therVal);
  delay(100);
}
