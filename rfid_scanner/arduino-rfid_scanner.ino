#include <SPI.h>
#include <MFRC522.h>
#include <WiFiS3.h>
#include "Button2.h"
#include <ArduinoJson.h>
#include "Matrix.h"

#define BUTTON_PIN D2
#define LED_PIN 8
#define LED_WIFI_PIN 7
#define RST_PIN 9
#define SS_PIN 10
#define LED_TIME_ON 5000  // 5 seconds (in milliseconds)
#define LOCATION_MAX 5
#define LOCATION_DEFAULT 1

struct WifiCredential {
  const char* ssid;
  const char* password;  // Can be NULL or empty string
};

// secrets.h should define the following:
//
// WifiCredential credentials[] = {};
#include "secrets.h"

const int credentialCount = sizeof(credentials) / sizeof(credentials[0]);

// State
int location = LOCATION_DEFAULT;
unsigned long readAt = 0;
bool ledOn = false;
String lastUid = "";

// const char *server = "192.168.5.213";
// const int port = 3000;
const char* server = "atm-clv-37eca624ed8b.herokuapp.com";
const int port = 80;
WiFiClient client;

// Matrix
Matrix matrix;

// Button
Button2 button;
void handleClick(Button2& b) {
  Serial.println("Button clicked");

  location += 1;
  if (location > LOCATION_MAX) {
    location = 0;
  }

  Serial.print("Location updated to ");
  Serial.println(location, DEC);

  matrix.update(location);
}

// RFID
MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(115200);
  delay(2000);
  while (!Serial)
    ;

  matrix.update(location);

  wifiConnect();

  button.begin(BUTTON_PIN);
  button.setTapHandler(handleClick);

  SPI.begin();
  mfrc522.PCD_Init();

  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_WIFI_PIN, OUTPUT);
  blink(LED_PIN, 2000);
  blink(LED_WIFI_PIN, 2000);
}

void loop() {
  button.loop();
  detectRFID();
  updateLED();
}

void sendPostRequest(String uid, int location) {
  if (client.connect(server, port)) {
    // JSON payload
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["id"] = uid;
    jsonDoc["loc"] = location;
    jsonDoc["at"] = "2025-07-06T15:00:00Z";

    String jsonData;
    serializeJson(jsonDoc, jsonData);

    // Build HTTP request
    client.println("POST /api/tracking_events HTTP/1.1");  // Replace with your API endpoint
    client.print("Host: ");
    client.println(server);
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(jsonData.length());
    client.println();          // Empty line before body
    client.println(jsonData);  // JSON data

    // Serial.println("POST request sent!");
  } else {
    // Serial.println("Connection failed!");
  }

  // Read response
  while (client.available()) {
    String response = client.readString();
    // Serial.println(response);
  }

  client.stop();  // Close connection
}

void wifiConnect() {
  bool connected = false;

  for (int i = 0; i < credentialCount; i++) {
    Serial.print("Attempting to connect to SSID=");
    Serial.print(credentials[i].ssid);

    if (credentials[i].password == nullptr || credentials[i].password[0] == '\0') {
      WiFi.begin(credentials[i].ssid);
    } else {
      WiFi.begin(credentials[i].ssid, credentials[i].password);
    }

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10) {
      delay(1000);
      Serial.print(".");
      attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected to SSID=");
      Serial.print(WiFi.SSID());
      Serial.print(", IP Address=");
      Serial.println(WiFi.localIP());
      connected = true;
      break;
    } else {
      Serial.println("Failed to connect.");
    }
  }

  if (!connected) {
    Serial.println("Could not connect to any known networks.");
  }
}

void updateLED() {
  if (ledOn && digitalRead(LED_PIN) == LOW) {
    digitalWrite(LED_PIN, HIGH);
  } else if (!ledOn && digitalRead(LED_PIN) == HIGH) {
    digitalWrite(LED_PIN, LOW);
  }

  if (readAt > 0 && millis() - readAt > LED_TIME_ON) {
    ledOn = false;
    readAt = 0;
    lastUid = "";
  }
}

void detectRFID() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String uidStr = getUIDString(mfrc522.uid.uidByte, mfrc522.uid.size);
    if (uidStr == lastUid) {
      return;
    }
    lastUid = uidStr;

    Serial.print("Scanned RFID ");
    Serial.print(lastUid);
    Serial.print(" at location ");
    Serial.println(location, DEC);

    mfrc522.PICC_HaltA();

    sendPostRequest(lastUid, location);

    readAt = millis();
    ledOn = true;

    Serial.print("Activating for ");
    Serial.print(LED_TIME_ON / 1000);
    Serial.println(" seconds");
  }
}

String getUIDString(byte* uid, byte length) {
  String uidStr = "";
  for (byte i = 0; i < length; i++) {
    if (uid[i] < 0x10) uidStr += "0";  // Ensure two-digit formatting
    uidStr += String(uid[i], HEX);
  }
  return uidStr;
}

void blink(int pin, int ms) {
  unsigned long now = millis();
  while (millis() - now < ms) {
    digitalWrite(pin, HIGH);
    delay(100);
    digitalWrite(pin, LOW);
    delay(100);
  }
  digitalWrite(pin, LOW);
}