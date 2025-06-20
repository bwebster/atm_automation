/*
  Controller ESP32          ⇆          ESP32 running WLED
  -------------------------------------------------------
     TX 17  ────────────────▶  RX
     RX 16  ◀───────────────  TX
     GND    ────────────────  GND
*/

#include <Arduino.h>
#include <DYPlayerArduino.h> // External: 
#include <ArduinoJson.h> // External: https://github.com/bblanchon/ArduinoJson v7.3.0

// Pins used to communicate with Arduino (scanner)
constexpr uint8_t TX_PIN = 19;
constexpr uint8_t RX_PIN = 18;

// Pins used to communicate with mp3 player
HardwareSerial MP3Serial(2);
constexpr uint8_t MP3_TX_PIN = 33;
constexpr uint8_t MP3_RX_PIN = 25;
constexpr uint32_t MP3_BAUD = 9600;

// ── UART to WLED ──────────────────────────────────────────────
HardwareSerial WLED(1);             // UART1  (RX16 / TX17)
constexpr uint32_t WLED_BAUD = 115200;
constexpr uint8_t  WLED_TX  = 17;   // connect to WLED RX
constexpr uint8_t  WLED_RX  = 16;   // connect to WLED TX

// ── Effect IDs (hard-coded) ──────────────────────────────────
constexpr uint16_t SOLID_ID =   0;
constexpr uint16_t BLURZ_ID = 163;
constexpr uint16_t BOUNCING_BALLS_ID = 92;
constexpr uint16_t CHASE_ID = 28;

// ── Low-level JSON send helper ───────────────────────────────
void sendJson(const JsonDocument& doc) {
  serializeJson(doc, WLED);
  WLED.println();                   // newline terminates the frame
}

// ── High-level LED helpers ───────────────────────────────────
void turnOn(uint16_t effectId) {
  StaticJsonDocument<64> j;
  j["on"] = true;
  j["seg"].createNestedObject()["fx"] = effectId;
  sendJson(j);
}

void turnOff() {
  StaticJsonDocument<16> j;
  j["on"] = false;
  sendJson(j);
}

void changeEffect(uint16_t effectId) {
  StaticJsonDocument<32> j;
  j["seg"].createNestedObject()["fx"] = effectId;
  sendJson(j);
}

void changeColor(uint8_t r, uint8_t g, uint8_t b) {
  StaticJsonDocument<64> j;
  JsonObject seg0 = j.createNestedArray("seg").createNestedObject();
  JsonArray  col0 = seg0.createNestedArray("col").createNestedArray();
  col0.add(r);
  col0.add(g);
  col0.add(b);
  sendJson(j);
}

void turnOnPreset(uint8_t id)
{
  StaticJsonDocument<64> j;
  j["on"] = true;
  j["ps"] = id;
  sendJson(j);
}

DY::Player player(&MP3Serial);
int track = 1;
int numTracks = 3;
bool last = LOW;
bool playing = false;

void setup() {
  Serial.begin(115200);

  Serial.println("Setting up pins");
  pinMode(TX_PIN, OUTPUT);
  pinMode(RX_PIN, INPUT_PULLDOWN);
  digitalWrite(TX_PIN, HIGH);
  
  Serial.println("Setting up WLED");
  WLED.begin(WLED_BAUD, SERIAL_8N1, WLED_RX, WLED_TX);
  delay(200);

  Serial.println("Testing LEDs");
  turnOff();
  delay(100);
  turnOnPreset(1);
  delay(1000);
  turnOnPreset(2);
  delay(1000);
  turnOff();
  delay(100);

  Serial.println("Initializing mp3 module");
  MP3Serial.begin(MP3_BAUD, SERIAL_8N1, MP3_RX_PIN, MP3_TX_PIN);
  delay(800);
  player.begin();
  player.setVolume(25);        // 0…30
}

void loop() {
  bool cur = digitalRead(RX_PIN);
  if (last == LOW && cur == HIGH) {
    Serial.println("Received HIGH from arduino");

    turnOnPreset(1);
    delay(100);
    player.playSpecified(1);
    delay(5000);
    turnOff();

    Serial.println("Sending done signal (HIGH)");
    digitalWrite(TX_PIN, LOW);
    delay(200);
    digitalWrite(TX_PIN, HIGH);

    Serial.println("Done");
  }
  last = cur;

//  Serial.println(F("→ turnOn(Bouncing Balls)"));
//   changeColor(0, 255, 0); // Green
//   turnOn(BOUNCING_BALLS_ID);
//   delay(10'000);

//  Serial.println(F("→ turnOn(Chase)"));
//  changeColor(0, 0, 255); // Blue
//   turnOn(CHASE_ID);
//   delay(10'000);

//   Serial.println(F("→ changeEffect(Solid)"));
//   changeColor(255, 255, 255); // White
//   changeEffect(SOLID_ID);
//   delay(5000);

//   Serial.println(F("→ turnOff()"));
//   turnOff();

//   Serial.println(F("Sleep"));
//   delay(5000);
}
