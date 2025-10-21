#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include <DFRobot_RGBLCD1602.h>
#include <ArduinoJson.h>

const char* ssid = "phone1";
const char* password = "phone123";

WebServer server(80);

DFRobot_RGBLCD1602 lcd(0x6B, 16, 2); 

const int LED_PIN = 2;
bool ledState = false;

void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

void handleStatus() {
  server.send(200, "application/json", String("{\"led\":") + (ledState ? "true" : "false") + "}");
}

void handleToggle() {
  ledState = !ledState;
  digitalWrite(LED_PIN, ledState ? LOW : HIGH);
  handleStatus();
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
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
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

  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/toggle", HTTP_POST, handleToggle);
  server.on("/api/screen", HTTP_POST, handleScreen);
  server.serveStatic("/", SPIFFS, "/index.html");
  server.serveStatic("/styles.css", SPIFFS, "/styles.css");
  server.serveStatic("/j.js", SPIFFS, "/j.js");
  server.onNotFound(handleNotFound);

  server.begin();
}

void loop() {
  server.handleClient();
}