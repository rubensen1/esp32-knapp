#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include <DFRobot_RGBLCD1602.h>
#include <ArduinoJson.h>
#include <FastLED.h>

// LED strip definisjon (fra første program)
#define LED_PIN 4
#define NUM_LEDS 38
CRGB leds[NUM_LEDS];


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

// spiffs lagringssted
const char* dataFile = "/data.json";
const char* qstFile = "/questions.json";

int questionIndex = 0;
DynamicJsonDocument activeQuestionsDoc(2048); // Holder aktive spørsmål med ID-er
JsonArray activeQuestions;

// WiFi credentials
const char* ssid = "phone1";
const char* password = "phone123";

// WebServer og LCD
WebServer server(80);
DFRobot_RGBLCD1602 lcd(0x6B, 16, 2);

void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

// ✅ Funksjon for å legge til spørsmål i SPIFFS
void handleAddQuestion() {
  if (server.hasArg("plain")) {
    String message = server.arg("plain");
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, message);
    
    if (!error && doc.containsKey("question")) {
      String question = doc["question"].as<String>();
      
      // Les eksisterende spørsmål
      File file = SPIFFS.open(qstFile, FILE_READ);
      DynamicJsonDocument qDoc(4096);
      
      if (file) {
        deserializeJson(qDoc, file);
        file.close();
      }
      
      JsonArray arr = qDoc.to<JsonArray>();
      
      // Finn neste ledige ID
      int nextId = 0;
      for (JsonObject obj : arr) {
        if (obj["id"].as<int>() >= nextId) {
          nextId = obj["id"].as<int>() + 1;
        }
      }
      
      // Legg til nytt spørsmål med ID
      JsonObject newQuestion = arr.createNestedObject();
      newQuestion["id"] = nextId;
      newQuestion["text"] = question;
      
      // Skriv tilbake til fil
      file = SPIFFS.open(qstFile, FILE_WRITE);
      if (file) {
        serializeJson(qDoc, file);
        file.close();
        
        // Send tilbake det nye spørsmålet med ID
        StaticJsonDocument<200> response;
        response["success"] = true;
        response["id"] = nextId;
        response["text"] = question;
        
        String responseStr;
        serializeJson(response, responseStr);
        server.send(200, "application/json", responseStr);
        
        Serial.println("Question added with ID " + String(nextId) + ": " + question);
      } else {
        server.send(500, "application/json", "{\"error\":\"Failed to write file\"}");
      }
    } else {
      server.send(400, "application/json", "{\"error\":\"Invalid JSON or missing question\"}");
    }
  } else {
    server.send(400, "application/json", "{\"error\":\"No data received\"}");
  }
}

// ✅ Funksjon for å hente alle spørsmål fra SPIFFS
void handleGetQuestions() {
  File file = SPIFFS.open(qstFile, FILE_READ);
  if (!file) {
    server.send(500, "application/json", "{\"error\":\"Failed to open file\"}");
    return;
  }
  
  String response = file.readString();
  file.close();
  
  // Sjekk at filen ikke er tom
  if (response.length() == 0) {
    response = "[]";  // Return empty array
  }
  
  Serial.println("Sending questions: " + response);
  server.send(200, "application/json", response);
}

// ✅ Funksjon for å slette alle spørsmål fra SPIFFS
void handleClearQuestions() {
  DynamicJsonDocument doc(512);
  doc.to<JsonArray>();  // Tom array
  
  File file = SPIFFS.open(qstFile, FILE_WRITE);
  if (file) {
    serializeJson(doc, file);
    file.close();
    server.send(200, "application/json", "{\"success\":true}");
    Serial.println("All questions cleared");
  } else {
    server.send(500, "application/json", "{\"error\":\"Failed to clear questions\"}");
  }
}

// Hjelper funksjon for å få timestamp
String getTimestamp() {
  unsigned long currentMillis = millis();
  unsigned long seconds = currentMillis / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  
  char timestamp[20];
  sprintf(timestamp, "%02lu:%02lu:%02lu", hours % 24, minutes % 60, seconds % 60);
  return String(timestamp);
}

void startScreen() {
  if (server.hasArg("plain")) {
    String message = server.arg("plain");
    Serial.println("Received data: " + message);
    
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, message);
    
    if (!error) {
      JsonArray questions = doc["questions"];
      
      Serial.print("Number of questions received: ");
      Serial.println(questions.size());
      
      // Lagre de aktive spørsmålene med ID-er
      activeQuestions = activeQuestionsDoc.to<JsonArray>();
      activeQuestions.clear();
      
      for (JsonVariant q : questions) {
        JsonObject qObj = activeQuestions.createNestedObject();
        
        // Sjekk om vi får riktige verdier
        if (q.containsKey("id") && q.containsKey("text")) {
          qObj["id"] = q["id"];
          qObj["text"] = q["text"];
          Serial.print("Added question ID: ");
          Serial.print(q["id"].as<int>());
          Serial.print(" Text: ");
          Serial.println(q["text"].as<String>());
        } else {
          Serial.println("Warning: Question missing id or text!");
        }
      }
      
      lcd.clear();
      
      // Reset question index
      questionIndex = 0;
      
      // Viser første spørsmål på LCD
      if (activeQuestions.size() > 0) {
        String question = activeQuestions[questionIndex]["text"].as<String>();
        int qid = activeQuestions[questionIndex]["id"].as<int>();
        Serial.println("Displaying question ID " + String(qid) + ": " + question);
        
        lcd.setCursor(0, 0);
        lcd.print(question.substring(0, 16));
        if (question.length() > 16) {
          lcd.setCursor(0, 1);
          lcd.print(question.substring(16, 32));
        }
      } else {
        Serial.println("No questions to display!");
      }
      
      server.send(200, "application/json", "{\"success\":true}");
    } else {
      Serial.println("JSON parsing error!");
      server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    }
  } else {
    server.send(400, "application/json", "{\"error\":\"No data received\"}");
  }
}

void initFile() {
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS mount failed!");
    return;
  }

  if (!SPIFFS.exists(dataFile)) {
    // Create empty JSON array
    DynamicJsonDocument doc(512);
    doc.to<JsonArray>();  // empty array

    File file = SPIFFS.open(dataFile, FILE_WRITE);
    serializeJson(doc, file);
    file.close();
    Serial.println("Created data.json");
  }

  if (!SPIFFS.exists(qstFile)) {
    // Create empty JSON array
    DynamicJsonDocument doc(512);
    doc.to<JsonArray>();  // empty array

    File file = SPIFFS.open(qstFile, FILE_WRITE);
    serializeJson(doc, file);
    file.close();
    Serial.println("Created questions.json");
  }
}


void appendEntry(int questionID, int result, const char* timestamp) {
  // Open file for reading
  File file = SPIFFS.open(dataFile, FILE_READ);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    Serial.println("Failed to parse JSON");
    return;
  }

  JsonArray arr = doc.as<JsonArray>();
  JsonObject newEntry = arr.createNestedObject();
  newEntry["questionID"] = questionID;
  newEntry["result"] = result;
  newEntry["timestamp"] = timestamp;

  // Open file for writing (overwrite)
  file = SPIFFS.open(dataFile, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  serializeJson(doc, file);
  file.close();

  Serial.print("Entry saved - QuestionID: ");
  Serial.print(questionID);
  Serial.print(" | Result: ");
  Serial.print(result);
  Serial.print(" | Timestamp: ");
  Serial.println(timestamp);
}

// Read and print all entries
void readEntries() {
  File file = SPIFFS.open(dataFile, FILE_READ);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    Serial.println("Failed to parse JSON");
    return;
  }

  JsonArray arr = doc.as<JsonArray>();
  Serial.println("=== All entries ===");
  for (JsonObject entry : arr) {
    Serial.print("QuestionID: "); Serial.print(entry["questionID"].as<int>());
    Serial.print(" | Result: "); Serial.print(entry["result"].as<int>());
    Serial.print(" | Timestamp: "); Serial.println(entry["timestamp"].as<const char*>());
  }
  Serial.println("==================");
}

// ✅ API endpoint for å hente statistikk
void handleGetData() {
  File file = SPIFFS.open(dataFile, FILE_READ);
  if (!file) {
    server.send(500, "application/json", "{\"error\":\"Failed to open file\"}");
    return;
  }
  
  String response = file.readString();
  file.close();
  
  server.send(200, "application/json", response);
}

void handleButtonPress(int button) {
  // Sjekk at vi har aktive spørsmål
  if (activeQuestions.size() == 0 || questionIndex >= activeQuestions.size()) {
    Serial.println("No active questions!");
    return;
  }
  
  // Få ID for gjeldende spørsmål
  int currentQuestionID = activeQuestions[questionIndex]["id"].as<int>();
  
  // Skru av de andre LED-ene basert på hvilken knapp som ble trykket
  if (button == 1) {
    for (int i = 0; i <= NUM_LEDS; i++) {
      leds[i] = CRGB (255, 0, 0);
    } FastLED.show();
    digitalWrite(green_led_pin, LOW);
    digitalWrite(yellow_led_pin, LOW);
  } else if (button == 2) {
    for (int i = 0; i <= NUM_LEDS; i++) {
      leds[i] = CRGB (0, 255, 0);
    } FastLED.show();
    digitalWrite(red_led_pin, LOW);
    digitalWrite(yellow_led_pin, LOW);
  } else {
    for (int i = 0; i <= NUM_LEDS; i++) {
      leds[i] = CRGB (0, 255, 255);
    } FastLED.show();
    digitalWrite(red_led_pin, LOW);
    digitalWrite(green_led_pin, LOW);
  }

  // Lagre resultatet med riktig question ID
  String timestamp = getTimestamp();
  appendEntry(currentQuestionID, button, timestamp.c_str());

  lcd.clear();
  delay(500);
  
  // Gå til neste spørsmål
  questionIndex++;
  
  // Vis neste spørsmål hvis det finnes flere
  if (questionIndex < activeQuestions.size()) {
    String nextQuestion = activeQuestions[questionIndex]["text"].as<String>();
    int nextQuestionID = activeQuestions[questionIndex]["id"].as<int>();
    
    Serial.println("Next question ID " + String(nextQuestionID) + ": " + nextQuestion);
    
    lcd.setCursor(0, 0);
    lcd.print(nextQuestion.substring(0, 16));
    if (nextQuestion.length() > 16) {
      lcd.setCursor(0, 1);
      lcd.print(nextQuestion.substring(16, 32));
    }
  } else {
    // Alle spørsmål besvart
    lcd.setCursor(0, 0);
    lcd.print("All done!");
    lcd.setCursor(0, 1);
    lcd.print("Thank you!");
    Serial.println("All questions answered!");
  }
  
  delay(500);
  
  // Skru på alle LED-ene igjen
  digitalWrite(green_led_pin, HIGH);
  digitalWrite(yellow_led_pin, HIGH);
  digitalWrite(red_led_pin, HIGH);
}


void setup() {
  // Knapper og LED setup
  pinMode(green_button_pin, INPUT);
  pinMode(yellow_button_pin, INPUT);
  pinMode(red_button_pin, INPUT);

  pinMode(green_led_pin, OUTPUT);
  pinMode(yellow_led_pin, OUTPUT);
  pinMode(red_led_pin, OUTPUT);

  digitalWrite(green_led_pin, HIGH);
  digitalWrite(yellow_led_pin, HIGH);
  digitalWrite(red_led_pin, HIGH);

  // Serial
  Serial.begin(115200);

  // led setup
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  
  // spiffs setup
  initFile();

  // Read all entries at startup
  readEntries();

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
  server.on("/api/screen", HTTP_POST, startScreen);
  server.on("/api/questions", HTTP_POST, handleAddQuestion);      // ✅ Legg til spørsmål
  server.on("/api/questions", HTTP_GET, handleGetQuestions);      // ✅ Hent spørsmål
  server.on("/api/questions", HTTP_DELETE, handleClearQuestions); // ✅ Slett spørsmål
  server.on("/api/data", HTTP_GET, handleGetData);                // ✅ Hent statistikk
  
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

  unsigned long currentTime = millis();
  
  // Styr LED-er basert på knapper med debounce
  if (currentTime - lastPress > deBounce) {
    if (red_button_state == 1) {
      handleButtonPress(1);  // Red = 1
      lastPress = currentTime;
      
    } else if (green_button_state == 1) {
      handleButtonPress(2);  // Green = 2
      lastPress = currentTime;
      
    } else if (yellow_button_state == 1) {
      handleButtonPress(3);  // Yellow = 3
      lastPress = currentTime;
    }
  }

  delay(20);
}