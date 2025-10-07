#include <Arduino.h>

const int ledPin = 2;

// put function declarations here:
int myFunction(int, int);

void setup() {
  // Start serial communication
  Serial.begin(115200);
  while (!Serial) { ; } // Vent til Serial er klar (for noen boards)
  
  // Sett LED-pinnen som output
  pinMode(ledPin, OUTPUT);
  
  Serial.println("ESP32 starter...");
}

void loop() {
  // Blink LED
  digitalWrite(ledPin, HIGH);
  Serial.println("LED ON");
  delay(1000); // vent 1 sekund
  
  digitalWrite(ledPin, LOW);
  Serial.println("LED OFF");
  delay(1000); // vent 1 sekund
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}