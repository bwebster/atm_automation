#include <SPI.h>
#include <WiFiS3.h>

#include "Fsm.h"         // External: https://github.com/jonblack/arduino-fsm v2.2.0
#include <MFRC522.h>     // External: https://github.com/miguelbalboa/rfid v1.4.12
#include <ArduinoJson.h> // External: https://github.com/bblanchon/ArduinoJson v7.3.0

#include "StringFifo.h"
#include "Matrix.h"

// Pins used to control automation
#define AUTOMATION_END_PIN 6    // Input pin when finished
#define AUTOMATION_START_PIN 7  // Output pin to start

// Configuration for LEDs that turn on after successful scan
#define LED_PIN 8         // Pin to trigger LEDs
#define LED_TIME_ON 5000  // How long, in milliseconds, to keep LEDs on

// SPI configuration for RFID scanner
#define RST_PIN 9   // Reset Pin
#define CS_PIN 10   // Chip Select
#define COPI_PIN 11 // Controller Out Peripheral In
#define CIPO_PIN 12 // Controller In, Peripheral Out
#define SCK_PIN 13  // Serial Clock

// Location number to send after successful scan
#define LOCATION 1 

// State
unsigned long automationStartedAt = 0;
bool ledOn = false;
String lastUid = "";
StringFifo<1> recentlyScanned; // Set the capacity to the number of recent scans to track.
                               // If an RFID tag is scanned, and it's in this list, it will be ignored.
                               // Prevents repeated scans.  MUST be at least 1.

// State machine
#define EVENT_TAG_SCANNED 0
#define EVENT_AUTOMATION_DONE 1

State state_led_off(&led_off_enter, NULL, NULL);
State state_led_on(&led_on_enter, NULL, NULL);
State state_automation_on(&automation_on_enter, NULL, NULL);
State state_automation_off(&automation_off_enter, NULL, NULL);

Fsm fsm_led(&state_led_off);
Fsm fsm_automation(&state_automation_off);

// Transitions
void led_off_enter() {
  Serial.println("Turn off LED");
  digitalWrite(LED_PIN, LOW);
}

void led_on_enter() {
  Serial.println("Turn on LED");
  digitalWrite(LED_PIN, HIGH);

  Serial.println("Send scan");
  sendPostRequest(lastUid);
  lastUid = "";
}

void automation_on_enter() {
  Serial.println("Trigger animation");
  digitalWrite(AUTOMATION_START_PIN, HIGH);
}

void automation_off_enter() {
  Serial.println("Turn off animation");
  digitalWrite(AUTOMATION_START_PIN, LOW);
}

void on_event_tag_scanned() {
  Serial.print("Scanned RFID ");
  Serial.print(lastUid);
  Serial.print(" at location ");
  Serial.println(LOCATION, DEC);

  if (recentlyScanned.full()) {
    recentlyScanned.drop();
  }
  recentlyScanned.push(lastUid);
}

struct WifiCredential {
  const char* ssid;
  const char* password;  // Can be NULL or empty string
};

// Add wifi credentials, to be tried in order until a successful connection.
// For local development you can define credentials[] in a secrets.h file to add
// you home network.  
//
// Sample secrets.h file:
//
//   #ifndef SECRETS_H
//   #define SECRETS_H
//   #define CREDENTIALS
//   static const WifiCredential credentials[] = {
//     {"MyNetwork", "MyPassword"},
//     {"Life.Church", nullptr},  // open network
//   };
//   #endif
//
// Then simply uncomment the #include "secrets.h" line below.
// #include "secrets.h"
#ifndef CREDENTIALS
static const WifiCredential credentials[] = {
  {"Life.Church", nullptr},  // open network
};
#endif
const int credentialCount = sizeof(credentials) / sizeof(credentials[0]);

// Configure the IP address and port for the server software.
// const char *server = "192.168.5.229";
// const int port = 3000;
const char* server = "atm-clv-37eca624ed8b.herokuapp.com";
const int port = 80;

WiFiClient client;
MFRC522 mfrc522(CS_PIN, RST_PIN);
Matrix matrix;

void setup() {
  Serial.begin(115200);
  delay(2000);
  while (!Serial)
    ;

  Serial.println("\n\nSetup start");

  fsm_led.add_transition(&state_led_off, &state_led_on, EVENT_TAG_SCANNED, &on_event_tag_scanned);
  fsm_led.add_timed_transition(&state_led_on, &state_led_off, LED_TIME_ON, NULL);
  fsm_automation.add_timed_transition(&state_automation_on, &state_automation_off, LED_TIME_ON, NULL);

  matrix.letterDelay('W', 1000);
  wifiConnect();
  matrix.ok();

  matrix.letterDelay('S', 1000);
  SPI.begin();
  mfrc522.PCD_Init();
  matrix.ok();

  pinMode(LED_PIN, OUTPUT);
  pinMode(AUTOMATION_START_PIN, OUTPUT);
  pinMode(AUTOMATION_END_PIN, INPUT);

  digitalWrite(AUTOMATION_START_PIN, LOW);
  digitalWrite(AUTOMATION_END_PIN, LOW);

  matrix.number(LOCATION);
}

void loop() {
  fsm_led.run_machine();
  fsm_automation.run_machine();

  detectRFID();
  detectAutomationDone();
}

void detectAutomationDone() {
  if (digitalRead(AUTOMATION_END_PIN) == HIGH) {
    fsm_automation.trigger(EVENT_AUTOMATION_DONE);
  }
}

void sendPostRequest(String uid) {
  if (client.connect(server, port)) {
    // JSON payload
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["id"] = uid;
    jsonDoc["loc"] = LOCATION;
    // hard coded for testing purposes
    // jsonDoc["at"] = "2025-07-06T15:00:00Z";

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

    Serial.println("POST request sent!");
  } else {
    Serial.println("Connection failed!");
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
      delay(2000);
      Serial.print("\nConnected to SSID=");
      Serial.print(WiFi.SSID());
      Serial.print(", IP Address=");
      Serial.print(WiFi.localIP());
      Serial.print(", Gateway=");
      Serial.print(WiFi.gatewayIP());
      Serial.println("");
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

void detectRFID() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String uidStr = getUIDString(mfrc522.uid.uidByte, mfrc522.uid.size);
    if (recentlyScanned.contains(uidStr)) {
      return;
    }
    lastUid = uidStr;
  
    mfrc522.PICC_HaltA();

    fsm_led.trigger(EVENT_TAG_SCANNED);
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