#include <SPI.h>
#include <WiFiS3.h>

#include "Fsm.h"          // External: https://github.com/jonblack/arduino-fsm v2.2.0
#include <MFRC522.h>      // External: https://github.com/miguelbalboa/rfid v1.4.12
#include <ArduinoJson.h>  // External: https://github.com/bblanchon/ArduinoJson v7.3.0

#include "StringFifo.h"
#include "Matrix.h"
#include "WifiCredentials.h"
#include "Automation.h"

#include "DigitalSignalAutomation.h"
DigitalSignalAutomation automation;

// #include "SoundAutomation.h"
// SoundAutomation automation;

// Configuration for LEDs that turn on after successful scan
#define LED_PIN 8          // Pin to trigger LEDs
#define LED_TIME_ON 5'000  // How long, in milliseconds, to keep LEDs on

// SPI configuration for RFID scanner
#define RST_PIN 9    // Reset Pin
#define CS_PIN 10    // Chip Select
#define COPI_PIN 11  // Controller Out Peripheral In
#define CIPO_PIN 12  // Controller In, Peripheral Out
#define SCK_PIN 13   // Serial Clock

// Time to wait before clearning scan history.
// Set to 0 to not automatically clear.
// If set to 0, you cannot double scan.
#define CLEAR_HISTORY_AFTER_MS 30'000

// Location number to send after successful scan
#define LOCATION 1

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
#include "secrets.h"
#ifndef CREDENTIALS
static const WifiCredential credentials[] = {
  { "Life.Church", nullptr },  // open network
};
#endif
const int credentialCount = sizeof(credentials) / sizeof(credentials[0]);

// Configure the IP address and port for the server software.
// const char *server = "192.168.5.229";
// const int port = 3000;
const char* server = "atm-clv-37eca624ed8b.herokuapp.com";
const int port = 80;

// State
unsigned long automationStartedAt = 0;
bool ledOn = false;
String lastUid = "";
unsigned long lastScanAt = 0;
StringFifo<1> recentlyScanned;  // Set the capacity to the number of recent scans to track.
                                // If an RFID tag is scanned, and it's in this list, it will be ignored.
                                // Prevents repeated scans.  MUST be at least 1.

// State machine
// #define EVENT_TAG_SCANNED 0
// #define EVENT_AUTOMATION_START 1
// #define EVENT_AUTOMATION_DONE 2

// Events
const uint8_t event_tag_scanned = 0;
const uint8_t event_acknowledged = 1;
const uint8_t event_automation_started = 2;
const uint8_t event_automation_ended = 3;
const uint8_t event_automation_timed_out = 4;

// States
State ready(&state_ready_on_enter, &state_ready_on, &state_ready_on_exit);
State scanned(&state_scanned_on_enter, &state_scanned_on, &state_scanned_on_exit);
State waiting(&state_waiting_on_enter, &state_waiting_on, &state_waiting_on_exit);

// State machine
Fsm scanner(&ready);

// Transitions
void state_ready_on_enter() {
  Serial.println("FSM ->ready");
  disable_leds();
}

void state_ready_on() {
  String uid = read_next_rfid();
  if (uid != "") {
    lastUid = uid;
    scanner.trigger(event_tag_scanned);
  }

  clear_recent_scans();
}

void state_ready_on_exit() {
  Serial.println("FSM ready->");
}

void state_scanned_on_enter() {
  Serial.println("FSM ->scanned");

  enable_leds();

  automation.run(&automation_callback);

  track_scan(lastUid);
  lastUid = "";
}

void state_scanned_on() {
  automation.update();

  scanner.trigger(event_automation_started);
}

void state_scanned_on_exit() { 
  Serial.println("FSM scanned->");

  // disable_leds();  wait for animation to finish
}

void state_waiting_on_enter() {
  Serial.println("FSM ->waiting");
}

void state_waiting_on() {
  automation.update();
}

void state_waiting_on_exit() {
  Serial.println("FSM waiting->");
}

void automation_callback() {
  Serial.println("Automation is done. Triggering event event_automation_ended");
  scanner.trigger(event_automation_ended);
}

// State transitions
void on_event_tag_scanned() {
  Serial.print("Scanned tag ");
  Serial.println(lastUid);

  lastScanAt = millis();

  if (recentlyScanned.full()) {
    recentlyScanned.drop();
  }
  recentlyScanned.push(lastUid);
};

void on_event_acknowledged() {}
void on_event_automation_started(){}
void on_event_automation_ended(){}

void on_waiting_timed_out() {
  Serial.println("[Timer] timed out in waiting state");
}

void on_scanned_timed_out() { 
  Serial.println("[Timer] timed out in scanned state");
}

WiFiClient client;
MFRC522 mfrc522(CS_PIN, RST_PIN);
Matrix matrix;

void setup() {
  Serial.begin(115200);
  delay(2000);
  while (!Serial)
    ;

  Serial.println("\n\nSetup start");

  // ready -> scanned
  scanner.add_transition(&ready, &scanned, event_tag_scanned, &on_event_tag_scanned);

  // scanned -> waiting
  scanner.add_transition(&scanned, &waiting, event_automation_started, &on_event_acknowledged);
  scanner.add_timed_transition(&scanned, &waiting, 5'000, &on_scanned_timed_out);

  // waiting -> ready
  scanner.add_transition(&waiting, &ready, event_automation_ended, &on_event_automation_ended);
  scanner.add_timed_transition(&waiting, &ready, 15'000, &on_waiting_timed_out);

  // Wifi setup
  matrix.letterDelay('W', 1000);
  wifi_connect();
  matrix.ok();

  // Scanner setup
  matrix.letterDelay('S', 1000);
  SPI.begin();
  mfrc522.PCD_Init();
  matrix.ok();

  pinMode(LED_PIN, OUTPUT);

  automation.setup();

  matrix.number(LOCATION);
}

void loop() {
  scanner.run_machine();
}

void enable_leds() {
  Serial.println("[Action] enable LEDs");
  digitalWrite(LED_PIN, HIGH);
}

void disable_leds() {
  Serial.println("[Action] disable LEDs");
  digitalWrite(LED_PIN, LOW);
}

void clear_recent_scans() {
  if (CLEAR_HISTORY_AFTER_MS > 0 && lastScanAt > 0 && millis() - lastScanAt > CLEAR_HISTORY_AFTER_MS) {
    Serial.println("[Action] clearning recent scans");
    lastScanAt = 0;
    while (!recentlyScanned.empty()) {
      recentlyScanned.drop();
    }
  }
}

bool connect_exp_backoff(WiFiClient& client, const char* server, uint16_t port, int maxRetries = 3, int initialDelayMs = 20) {
  int delayMs = initialDelayMs;

  for (int attempt = 1; attempt <= maxRetries; attempt++) {
    if (client.connect(server, port)) {
      return true;
    }

    Serial.print("Connection failed. Backing off for ");
    Serial.print(delayMs);
    Serial.println("ms...");
    delay(delayMs);
    delayMs *= 2;  // Exponential backoff
  }

  Serial.println("All connection attempts failed!");
  return false;
}

void track_scan(String uid) {
  if (connect_exp_backoff(client, server, port)) {
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

    Serial.println("[Action] tracked scan via HTTP POST");
  } else {
    Serial.println("[Action] failed to track scan - connection failed!");
  }

  // Read response
  while (client.available()) {
    String response = client.readString();
    // Serial.println(response);
  }

  client.stop();  // Close connection
}

void wifi_connect() {
  for (int retries = 0; retries < 5; retries++) {
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
        return;
      } else {
        Serial.println("Failed to connect.");
      }
    }
  }
  Serial.println("Could not connect to any known networks.");
}

String read_next_rfid() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String uidStr = uid_string(mfrc522.uid.uidByte, mfrc522.uid.size);
    if (recentlyScanned.contains(uidStr)) {
      return "";
    }

    mfrc522.PICC_HaltA();

    return uidStr;
  }
  return "";
}

String uid_string(byte* uid, byte length) {
  String uidStr = "";
  for (byte i = 0; i < length; i++) {
    if (uid[i] < 0x10) uidStr += "0";  // Ensure two-digit formatting
    uidStr += String(uid[i], HEX);
  }
  return uidStr;
}