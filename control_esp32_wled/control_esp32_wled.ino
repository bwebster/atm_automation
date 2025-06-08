/*
  Controller ESP32          ⇆          ESP32 running WLED
  -------------------------------------------------------
     TX 17  ────────────────▶  RX
     RX 16  ◀───────────────  TX
     GND    ────────────────  GND
*/

#include <Arduino.h>
#include <ArduinoJson.h> // External: https://github.com/bblanchon/ArduinoJson v7.3.0

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

// ── Demo sequence ────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  WLED.begin(WLED_BAUD, SERIAL_8N1, WLED_RX, WLED_TX);
}

void loop() {
 Serial.println(F("→ turnOn(Bouncing Balls)"));
  changeColor(0, 255, 0); // Green
  turnOn(BOUNCING_BALLS_ID);
  delay(10'000);

 Serial.println(F("→ turnOn(Chase)"));
 changeColor(0, 0, 255); // Blue
  turnOn(CHASE_ID);
  delay(10'000);

  Serial.println(F("→ changeEffect(Solid)"));
  changeColor(255, 255, 255); // White
  changeEffect(SOLID_ID);
  delay(5000);

  Serial.println(F("→ turnOff()"));
  turnOff();

  Serial.println(F("Sleep"));
  delay(5000);
}
