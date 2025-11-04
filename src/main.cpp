#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include <DFRobot_RGBLCD1602.h>
#include <ArduinoJson.h>

// LED strip definisjon (fra første program)
#define LED_PIN 4
#define NUM_LEDS 20

// Knapp pins
const int green_button_pin = 34;
const int yellow_button_pin = 36;
const int red_button_pin = 39;

// LED pins
const int green_led_pin = 17;
const int yellow_led_pin = 18;
const int red_led_pin = 19;

// Knapp states
int green_button_state = 1;
int yellow_button_state = 1;
int red_button_state = 1;

// debounce ting
unsigned long lastPress = 0;
const unsigned long deBounce = 500;

// WiFi credentials
const char* ssid = "phone1";
const char* password = "phone123";

// WebServer og LCD
WebServer server(80);
DFRobot_RGBLCD1602 lcd(0x6B, 16, 2);

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

void handleButtonPress(int &button) {
  delay(500);
}


void setup() {
  // Knapper og LED setup
  pinMode(green_button_pin, INPUT);
  pinMode(yellow_button_pin, INPUT);
  pinMode(red_button_pin, INPUT);

  pinMode(green_led_pin, OUTPUT);
  pinMode(yellow_led_pin, OUTPUT);
  pinMode(red_led_pin, OUTPUT);

  // Serial
  Serial.begin(115200);
  delay(1000);

  Serial.println("ESP32 starter");
  
  // I2C og LCD
  Wire.begin();
  lcd.init();
  lcd.setRGB(0, 0, 255);
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
  Serial.println("Starting WiFi connection...");
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 20) {
    delay(300);
    Serial.print(".");
    timeout++;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nWiFi connection failed!");
    Serial.println("Check SSID and password");
    return;
  }

  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

  // mDNS
  if (MDNS.begin("esp32")) {
    Serial.println("mDNS responder started: http://esp32.local/");
  }

  // WebServer routes
  server.on("/api/screen", HTTP_POST, handleScreen);
  server.serveStatic("/", SPIFFS, "/index.html");
  server.serveStatic("/styles.css", SPIFFS, "/styles.css");
  server.serveStatic("/script.js", SPIFFS, "/script.js");
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("WebServer startet");
}

void loop() {
  // Håndter webserver
  server.handleClient();

  // Les knappestatus
  int green_button_state = digitalRead(green_button_pin);
  int yellow_button_state = digitalRead(yellow_button_pin);
  int red_button_state = digitalRead(red_button_pin);



  // Styr LED-er basert på knapper
  if (green_button_state == 1) {
    digitalWrite(green_led_pin, LOW);
    digitalWrite(yellow_led_pin, HIGH);
    digitalWrite(red_led_pin, HIGH);

  } else if (yellow_button_state == 1) {
    digitalWrite(green_led_pin, HIGH);
    digitalWrite(yellow_led_pin, LOW);
    digitalWrite(red_led_pin, HIGH);

  } else if (red_button_state == 1) {
    digitalWrite(green_led_pin, HIGH);
    digitalWrite(yellow_led_pin, HIGH);
    digitalWrite(red_led_pin, LOW);
    
  } else {
    digitalWrite(green_led_pin, HIGH);
    digitalWrite(yellow_led_pin, HIGH);
    digitalWrite(red_led_pin, HIGH);
  }

  delay(20);
}