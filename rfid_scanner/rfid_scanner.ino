#include <SPI.h>
#include <WiFiS3.h>

#include "Fsm.h"          // External: https://github.com/jonblack/arduino-fsm v2.2.0
#include <MFRC522.h>      // External: https://github.com/miguelbalboa/rfid v1.4.12
#include <ArduinoJson.h>  // External: https://github.com/bblanchon/ArduinoJson v7.3.0

#include "StringFifo.h"
#include "Matrix.h"
#include "WifiCredentials.h"
#include "config.h"


// Configuration for LEDs that turn on after successful scan
#define LED_PIN 8    // Pin to trigger LEDs

// SPI configuration for RFID scanner
#define RST_PIN 9    // Reset Pin
#define CS_PIN 10    // Chip Select
#define COPI_PIN 11  // Controller Out Peripheral In
#define CIPO_PIN 12  // Controller In, Peripheral Out
#define SCK_PIN 13   // Serial Clock

const int credentialCount = sizeof(credentials) / sizeof(credentials[0]);

// State
unsigned long automationStartedAt = 0;
bool ledOn = false;
String lastUid = "";
unsigned long lastScanAt = 0;
// Set the capacity to the number of recent scans to track.
// If an RFID tag is scanned, and it's in this list, it will be ignored.
// Prevents repeated scans.  MUST be at least 1.
StringFifo<RECENT_SCAN_HISTORY_SIZE> recentlyScanned; 

// If tracking fails to send, implement non-blocking exponential backoff to keep trying
unsigned long trackScanLastAttempt = 0;
int trackScanRetryDelayMS = 2; // ms
const int trackScanMaxDelay = 3000; // max ms to wait between retry attempts

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

  trackScanLastAttempt = 0; // time of last attempt
  trackScanRetryDelayMS = 2;  // delay ms
}

void state_scanned_on() {
  automation.update();

  if (lastUid == "") return;  // Nothing to retry

  unsigned long now = millis();
  if (now - trackScanLastAttempt >= trackScanRetryDelayMS) {
    Serial.println("[Action] Attempting to send scan");

    bool success = track_scan(lastUid);
    trackScanLastAttempt = now;

    if (success) {
      Serial.println("[Action] Scan sent successfully");
      lastUid = "";

      scanner.trigger(event_automation_started);
    } else {
      Serial.println("[Action] Scan failed, backing off");

      trackScanLastAttempt = min(trackScanLastAttempt * 2, trackScanMaxDelay);
    }
  }
}

void state_scanned_on_exit() { 
  Serial.println("FSM scanned->");
}

void state_waiting_on_enter() {
  Serial.println("FSM ->waiting");
}

void state_waiting_on() {
  automation.update();
}

void state_waiting_on_exit() {
  Serial.println("FSM waiting->");

  automation.cancel();
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
  Serial.println("[Timer] timed out while waiting for automation to end");
}

void on_scanned_timed_out() { 
  Serial.println("[Timer] timed out while processing scan");
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

  // Pins
  pinMode(LED_PIN, OUTPUT);

  // FSM
  // ready -> scanned
  scanner.add_transition(&ready, &scanned, event_tag_scanned, &on_event_tag_scanned);
  // scanned -> waiting
  scanner.add_transition(&scanned, &waiting, event_automation_started, &on_event_acknowledged);
  scanner.add_timed_transition(&scanned, &waiting, SCAN_TIMEOUT_MS, &on_scanned_timed_out);
  // waiting -> ready
  scanner.add_transition(&waiting, &ready, event_automation_ended, &on_event_automation_ended);
  scanner.add_timed_transition(&waiting, &ready, AUTOMATION_TIMEOUT_MS, &on_waiting_timed_out);

  // Wifi setup
  wifi_connect();
  blink(1);

  // Scanner setup
  SPI.begin();
  mfrc522.PCD_Init();
  matrix.number(LOCATION);
  blink(2);

  automation.setup();
  blink(3);
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

bool track_scan(String uid) {
  if (client.connect(server, port)) {
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
    return true;
  } else {
    Serial.println("[Action] failed to track scan - connection failed!");
    return false;
  }

  // Read response
  while (client.available()) {
    String response = client.readString();
    // Serial.println(response);
  }

  client.stop();  // Close connection

  return true;
}

void wifi_connect() {
  for (int i = 0; i < credentialCount; i++) {
    Serial.print("Attempting to connect to SSID=");
    Serial.println(credentials[i].ssid);

    int delayMS = 20;
    for (int retries = 0; retries < 5; retries++) {
      if (credentials[i].password == nullptr || credentials[i].password[0] == '\0') {
        WiFi.begin(credentials[i].ssid);
      } else {
        WiFi.begin(credentials[i].ssid, credentials[i].password);
      }

      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 10) {
        delay(500);
        attempts++;
      }

      if (WiFi.status() == WL_CONNECTED) {
        attempts = 0;
        while (WiFi.localIP() == "0.0.0.0" && attempts < 10) {
          delay(200);
          attempts++;
        }

        Serial.print("Connected to SSID=");
        Serial.print(WiFi.SSID());
        Serial.print(", IP Address=");
        Serial.print(WiFi.localIP());
        Serial.print(", Gateway=");
        Serial.print(WiFi.gatewayIP());
        Serial.println("");
        return;
      } else {
        Serial.println("Failed to connect");
      }

      Serial.print("Waiting ");
      Serial.print(delayMS);
      Serial.println("ms before trying again");
      delay(delayMS);
      delayMS *= 2;
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

void blink(int times) {
  digitalWrite(LED_PIN, LOW);
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(120);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }
  delay(1000);
}