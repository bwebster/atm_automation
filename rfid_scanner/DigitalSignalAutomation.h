#ifndef DIGITAL_SIGNAL_AUTOMATION_H
#define DIGITAL_SIGNAL_AUTOMATION_H

#include "Automation.h"

const uint8_t TX_PIN = 5;  // TX to module; HIGH to start
const uint8_t RX_PIN = 4;  // RX from module; HIGH when done

class DigitalSignalAutomation : public Automation {
public:
  DigitalSignalAutomation()
    : inputMode(INPUT_PULLDOWN) {}

  DigitalSignalAutomation(uint8_t inputMode)
    : inputMode(inputMode) {}

  /* one-time hardware setup */
  void setup() override {
    Serial.println("Setting up digital signal automation");
    pinMode(TX_PIN, OUTPUT);
    pinMode(RX_PIN, inputMode);  // optional, only if RX is used
    digitalWrite(TX_PIN, LOW);   // ensure starting LOW
    last = LOW;
  }

  void run(DoneCb cb) override {
    Serial.println("[Action] reset TX to LOW before HIGH to create rising edge");

    // Generate a clean LOW-to-HIGH edge
    digitalWrite(TX_PIN, LOW);   // force LOW
    delay(10);                   // brief delay to ensure LOW state is latched
    digitalWrite(TX_PIN, HIGH);  // rising edge now happens
    last = LOW;                  // so that update() can catch the rising edge
    doneCb_ = cb;
    active_ = true;
  }

  /* poll peer in loop() */
  void update() override {
    if (!active_) return;

    bool now = digitalRead(TX_PIN);
    if (last == LOW && now == HIGH) {
      Serial.println("[Action] automation done - rising edge detected");
      active_ = false;
      digitalWrite(TX_PIN, LOW);  // reset signal so it’s ready for next run
      if (doneCb_) {
        DoneCb cb = doneCb_;
        doneCb_ = nullptr;
        cb();  // notify caller
      }
    }
    last = now;
  }

private:
  DoneCb doneCb_ = nullptr;
  bool active_ = false;
  bool last = LOW;  // updated default to match LOW starting state
  uint8_t inputMode;
};

#endif
