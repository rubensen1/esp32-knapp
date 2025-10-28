#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include <DFRobot_RGBLCD1602.h>
#include <ArduinoJson.h>
#include <FastLED.h>

#define LED_PIN 4
#define NUM_LEDS 20

const char* ssid = "phone1";
const char* password = "phone123";

WebServer server(80);

DFRobot_RGBLCD1602 lcd(0x6B, 16, 2); 

const int kulereLedPin = 2;
bool ledState = false;


CRGB leds[NUM_LEDS];

const int green_button_pin = 34;
const int yellow_button_pin = 36;
const int red_button_pin = 39;

const int sda_pin = 21;
const int scl_pin = 22;

const int green_led_pin = 17;
const int yellow_led_pin = 18;
const int red_led_pin = 19;

int green_button_state = 1;
int yellow_button_state = 1;
int red_button_state = 1;

void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

void handleScreen() {
  if (server.hasArg("plain")) {
    String message = server.arg("plain");
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, message);
    
    if (!error) {
      JsonArray questions = doc["questions"];
      lcd.clear();
      
      // Viser første spørsmål på LCD
      if (questions.size() > 0) {
        String question = questions[0];
        // Hvis spørsmålet er lengre enn 16 tegn, vis bare første 16
        lcd.setCursor(0, 0);
        lcd.print(question.substring(0, 16));
        if (question.length() > 16) {
          lcd.setCursor(0, 1);
          lcd.print(question.substring(16, 32));
        }
      }
      
      server.send(200, "application/json", "{\"success\":true}");
    } else {
      server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    }
  } else {
    server.send(400, "application/json", "{\"error\":\"No data received\"}");
  }
}

void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  
  pinMode(green_button_pin, INPUT);
  pinMode(yellow_button_pin, INPUT);
  pinMode(red_button_pin, INPUT);

  pinMode(green_led_pin, OUTPUT);
  pinMode(yellow_led_pin, OUTPUT);
  pinMode(red_led_pin, OUTPUT);

  pinMode(sda_pin, OUTPUT);
  pinMode(scl_pin, OUTPUT);

  Serial.begin(115200);

  Serial.begin(115200);
  pinMode(kulereLedPin, OUTPUT);
  digitalWrite(kulereLedPin, HIGH);
  lcd.setRGB(0, 0, 255);

  lcd.init();            // Initialiser skjermen
  lcd.setCursor(0, 0);
  lcd.print("Hello world!");
  lcd.setCursor(0, 1);
  lcd.print("ESP32 DFRobot");

  // SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS mount failed!");
    return;
  }

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

  if (MDNS.begin("esp32")) {
    Serial.println("mDNS responder started: http://esp32.local/");
  }

  server.on("/api/screen", HTTP_POST, handleScreen);
  server.serveStatic("/", SPIFFS, "/index.html");
  server.serveStatic("/styles.css", SPIFFS, "/styles.css");
  server.serveStatic("/script.js", SPIFFS, "/script.js");
  server.onNotFound(handleNotFound);

  server.begin();
}

void loop() {
  server.handleClient();


  int green_button_state = digitalRead(green_button_pin);
  int yellow_button_state = digitalRead(yellow_button_pin);
  int red_button_state = digitalRead(red_button_pin);
  Serial.println("ballalalalal");
  

  if (green_button_state == 1) {
    Serial.println("Green button");
    digitalWrite(green_led_pin, LOW);
    digitalWrite(yellow_led_pin, HIGH);
    digitalWrite(red_led_pin, HIGH);

    for (int i = 0; i <= 19; i++) {
    leds[i] = CRGB (0, 255,0);
    }
    FastLED.show();

  } else if (yellow_button_state == 1) {
     Serial.println("Yellow button");
     digitalWrite(green_led_pin, HIGH);
     digitalWrite(yellow_led_pin, LOW);
     digitalWrite(red_led_pin, HIGH);

    for (int i = 0; i <= 19; i++) {
    leds[i] = CRGB (255, 255,0);
    }
    FastLED.show();

  } else if (red_button_state == 1) {
    Serial.println("Red button");
    digitalWrite(green_led_pin, HIGH);
    digitalWrite(yellow_led_pin, HIGH);
    digitalWrite(red_led_pin, LOW);

    for (int i = 0; i <= 19; i++) {
      leds[i] = CRGB (255, 0,0);
    }
    FastLED.show();
  }

  else {
    digitalWrite(green_led_pin, HIGH);
    digitalWrite(yellow_led_pin, HIGH);
    digitalWrite(red_led_pin, HIGH);
  }

  delay(20);
}