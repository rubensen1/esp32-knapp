#include <Arduino.h>

#define LED_PIN 4
#define NUM_LEDS 20

const int green_button_pin = 34;
const int yellow_button_pin = 36;
const int red_button_pin = 39;

const int green_led_pin = 17;
const int yellow_led_pin = 18;
const int red_led_pin = 19;

int green_button_state = 1;
int yellow_button_state = 1;
int red_button_state = 1;

void setup() {
  pinMode(green_button_pin, INPUT);
  pinMode(yellow_button_pin, INPUT);
  pinMode(red_button_pin, INPUT);

  pinMode(green_led_pin, OUTPUT);
  pinMode(yellow_led_pin, OUTPUT);
  pinMode(red_led_pin, OUTPUT);

  Serial.begin(115200);
}

void loop() {

  int green_button_state = digitalRead(green_button_pin);
  int yellow_button_state = digitalRead(yellow_button_pin);
  int red_button_state = digitalRead(red_button_pin);
  Serial.println("ballalalalal");
  

  if (green_button_state == 1) {
    Serial.println("Green button");
    digitalWrite(green_led_pin, LOW);
    digitalWrite(yellow_led_pin, HIGH);
    digitalWrite(red_led_pin, HIGH);

  } else if (yellow_button_state == 1) {
     Serial.println("Yellow button");
     digitalWrite(green_led_pin, HIGH);
     digitalWrite(yellow_led_pin, LOW);
     digitalWrite(red_led_pin, HIGH);

  } else if (red_button_state == 1) {
    Serial.println("Red button");
    digitalWrite(green_led_pin, HIGH);
    digitalWrite(yellow_led_pin, HIGH);
    digitalWrite(red_led_pin, LOW);
  }

  else {
    digitalWrite(green_led_pin, HIGH);
    digitalWrite(yellow_led_pin, HIGH);
    digitalWrite(red_led_pin, HIGH);
  }

  delay(20);
}