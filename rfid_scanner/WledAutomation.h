#ifndef WLED_AUTOMATION_H
#define WLED_AUTOMATION_H

#include "Automation.h"
#include <SoftwareSerial.h>
#include <ArduinoJson.h> // External: https://github.com/bblanchon/ArduinoJson v7.3.0+

// UART-to-WLED (software)
#define TX_PIN 6    // UNO ➜ WLED RX
#define RX_PIN 5    // UNO ⇐ WLED TX
#define WLED_BAUD 115200
#define RUN_TIME_MS 10000 // How long to keep the lights on for

constexpr uint16_t WLED_NUM_PS = 4; // Number of presets, including setup() preset

// Effect IDs (hard-coded)
constexpr uint16_t SOLID_ID          =   0;
constexpr uint16_t BLINK_ID          =   1;
constexpr uint16_t BLURZ_ID          = 163;
constexpr uint16_t BOUNCING_BALLS_ID =  92;
constexpr uint16_t CHASE_ID          =  28;

// Pallette IDs
constexpr uint8_t AURORA_ID = 50;
constexpr uint8_t ATLANTICA_ID = 51;


class WledAutomation : public Automation {
public:
  WledAutomation() 
    : wledSerial(RX_PIN, TX_PIN) {};

  void setup() override {
    Serial.println("Setting up WLED automation");

    wledSerial.begin(WLED_BAUD); // software UART to WLED
    delay(200);

    turnOnStartUpCheck();
    delay(5000);
    turnOff();
  }

  void run(DoneCb cb) override {
    uint16_t preset = (num % WLED_NUM_PS) + 1;
    Serial.print("[Action] turning on preset ");
    Serial.println(preset);
    turnOnPreset(preset);
    num += 1;
    
    startAt = millis();
    doneCb_ = cb;
    active_ = true;
  }

  void update() override {
    if (!active_) return;

    if (millis() - startAt < RUN_TIME_MS) {
      return;
    }

    Serial.println("[Action] hit time limit, turning off LEDs");
    wledSerial.println(4);
    wledSerial.println("{\"on\":false}");
    // turnOff();

    active_ = false;
    if (doneCb_) {
      DoneCb cb = doneCb_;  // copy in case cb restarts us
      doneCb_ = nullptr;
      cb();  // notify caller exactly once
    }
  }

private:
  DoneCb doneCb_ = nullptr;
  bool active_ = false;
  SoftwareSerial wledSerial;
  unsigned long startAt;
  bool last = LOW;
  int num = 0;

// ── Tiny helper to send JSON over the software UART ────────────────
void sendJson(const JsonDocument& doc)
{
  serializeJson(doc, wledSerial);
  wledSerial.println();                     // newline terminates the frame
}

// ── High-level LED helpers ──────────────────────────────────────────
void turnOn(uint16_t effectId)
{
  StaticJsonDocument<64> j;
  j["on"] = true;
  j["seg"].createNestedObject()["fx"] = effectId;
  sendJson(j);
}

void turnOnStartUpCheck()
{
  StaticJsonDocument<64> j;
  j["on"] = true;
  JsonObject seg0 = j.createNestedArray("seg").createNestedObject();
  seg0["fx"] = BLINK_ID;
  seg0["sx"] = 200;
  seg0["pal"] = 0;
  JsonArray  col0 = seg0.createNestedArray("col").createNestedArray();
  col0.add(255);
  col0.add(0);
  col0.add(0);
  sendJson(j);
}

void turnOnWithColor(uint16_t effectId, uint8_t r, uint8_t g, uint8_t b)
{
  StaticJsonDocument<64> j;
  j["on"] = true;
  JsonObject seg0 = j.createNestedArray("seg").createNestedObject();
  seg0["fx"] = effectId;
  seg0["pal"] = 0;
  JsonArray  col0 = seg0.createNestedArray("col").createNestedArray();
  col0.add(r);
  col0.add(g);
  col0.add(b);
  sendJson(j);
}

void turnOff()
{
  StaticJsonDocument<16> j;
  j["on"] = false;
  sendJson(j);
}

void turnOnPreset(uint8_t id)
{
  StaticJsonDocument<64> j;
  j["on"] = true;
  j["ps"] = id;
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
};

#endif