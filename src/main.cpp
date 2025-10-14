#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>

const char* ssid = "phone1";
const char* password = "phone123";

WebServer server(80);

DFRobot_LCD lcd(16, 2);  // 16x2 LCD

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

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.display();
  lcd.setCursor(0, 0);
  lcd.print("Ready!");

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

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

void handleScreen() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, body);

    if (error) {
      server.send(400, "text/plain", "Invalid JSON");
      return;
    }

    if (!doc.containsKey("questions")) {
      server.send(400, "text/plain", "Missing questions");
      return;
    }

    JsonArray arr = doc["questions"];
    lcd.clear();

    // Print first 2 lines only (since LCD is 16x2)
    for (int i = 0; i < arr.size() && i < 2; i++) {
      lcd.setCursor(0, i);
      lcd.print(arr[i].as<const char*>());
    }

    server.send(200, "text/plain", "Displayed on LCD");
    Serial.println("Displayed on LCD:");
    serializeJson(doc, Serial);
  } else {
    server.send(400, "text/plain", "Missing body");
  }
}