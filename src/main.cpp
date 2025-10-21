#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>

const char* ssid = "phone1";
const char* password = "phone123";

WebServer server(80);

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
  server.serveStatic("/", SPIFFS, "/index.html");
  server.serveStatic("/styles.css", SPIFFS, "/styles.css");
  server.serveStatic("/j.js", SPIFFS, "/j.js");
  server.onNotFound(handleNotFound);

  server.begin();
}

void loop() {
  server.handleClient();
}