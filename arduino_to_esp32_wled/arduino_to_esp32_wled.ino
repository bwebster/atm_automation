/*
   Arduino UNO         ⇆        ESP32 running WLED
   ----------------------------------------------------------------
           6  ────────────────▶  RX   (ESP32 pin tied to UART2 RX16)
           5  ◀────────────────  TX   (ESP32 pin tied to UART2 TX17)
          GND ─────────────────  GND

   NOTE 1  ▸  We use SoftwareSerial because the UNO’s only hardware port
              (Serial) is busy talking to the PC.
   NOTE 2  ▸  57 600 baud is a safe upper limit for SoftwareSerial; if you
              see garbled bytes drop to 38 400 or 19 200 on **both** ends.
*/

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>          // v7.3.0+

// ── UART-to-WLED (software) ─────────────────────────────────────────
constexpr uint8_t  WLED_TX = 6;   // UNO ➜ WLED RX
constexpr uint8_t  WLED_RX = 5;   // UNO ⇐ WLED TX
constexpr uint32_t WLED_BAUD = 115200;

SoftwareSerial WLED(WLED_RX, WLED_TX);   // RX, TX

// ── Effect IDs (hard-coded) ─────────────────────────────────────────
constexpr uint16_t SOLID_ID          =   0;
constexpr uint16_t BLURZ_ID          = 163;
constexpr uint16_t BOUNCING_BALLS_ID =  92;
constexpr uint16_t CHASE_ID          =  28;

// ── Tiny helper to send JSON over the software UART ────────────────
void sendJson(const JsonDocument& doc)
{
  serializeJson(doc, WLED);
  WLED.println();                     // newline terminates the frame
}

// ── High-level LED helpers ──────────────────────────────────────────
void turnOn(uint16_t effectId)
{
  StaticJsonDocument<64> j;
  j["on"] = true;
  j["seg"].createNestedObject()["fx"] = effectId;
  sendJson(j);
}

void turnOff()
{
  StaticJsonDocument<16> j;
  j["on"] = false;
  sendJson(j);
}

void changeEffect(uint16_t effectId)
{
  StaticJsonDocument<32> j;
  j["seg"].createNestedObject()["fx"] = effectId;
  sendJson(j);
}

void changeColor(uint8_t r, uint8_t g, uint8_t b)
{
  StaticJsonDocument<64> j;
  JsonObject seg0 = j.createNestedArray("seg").createNestedObject();
  JsonArray  col0 = seg0.createNestedArray("col").createNestedArray();
  col0.add(r);
  col0.add(g);
  col0.add(b);
  sendJson(j);
}

// ── Demo sequence ──────────────────────────────────────────────────
void setup()
{
  Serial.begin(115200);            // USB monitor (PC ↔ UNO)
  WLED.begin(WLED_BAUD);           // software UART to WLED
}

void loop()
{
  Serial.println(F("→ turnOn(Bouncing Balls)"));
  changeColor(0, 255, 0);          // green
  turnOn(BOUNCING_BALLS_ID);
  delay(10000);

  Serial.println(F("→ turnOn(Chase)"));
  changeColor(0, 0, 255);          // blue
  turnOn(CHASE_ID);
  delay(10000);

  Serial.println(F("→ changeEffect(Solid)"));
  changeColor(255, 255, 255);      // white
  changeEffect(SOLID_ID);
  delay(5000);

  Serial.println(F("→ turnOff()"));
  turnOff();

  Serial.println(F("Sleep"));
  delay(5000);
}
