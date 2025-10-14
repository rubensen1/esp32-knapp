#include "DFRobot_RGBLCD1602.h"

DFRobot_RGBLCD1602 lcd(0x6B, 16, 2); // RGB-adresse, 16x2 display

void setup() {
  lcd.init();            // Initialiser skjermen
  lcd.setCursor(0, 0);
  lcd.print("Hello world!");
  lcd.setCursor(0, 1);
  lcd.print("ESP32 DFRobot");
}

void loop() {
  // Slå RGB-baklys mellom rød og blå
  lcd.setRGB(255, 0, 0); // Rød
  delay(1000);
  lcd.setRGB(0, 0, 255); // Blå
  delay(1000);
}
