#ifndef DIGITAL_SIGNAL_AUTOMATION_H
#define DIGITAL_SIGNAL_AUTOMATION_H

#include "Automation.h"

const uint8_t TX_PIN = 5; // TX to module; HIGH to start
const uint8_t RX_PIN = 4; // RX from module; HIGH when done

class DigitalSignalAutomation : public Automation {
public:
   DigitalSignalAutomation(uint8_t inputMode)
    : inputMode(inputMode) {}

  /* one-time hardware setup */
  void setup() override {
    Serial.println("Setting up digital signal automation");
    pinMode(TX_PIN, OUTPUT);
    pinMode(RX_PIN, inputMode); // Pass either INPUT_PULLDOWN or INPUT_PULLUP to constructor
    digitalWrite(TX_PIN, LOW);
  }

  void run(DoneCb cb) override {
    Serial.println("[Action] set automation TX to HIGH");
    digitalWrite(TX_PIN, HIGH);         // signal out
    doneCb_ = cb;
    active_ = true;
  }

  /* poll peer in loop() */
  void update() override {
    if (!active_) return;

    bool now = digitalRead(TX_PIN);
    if (last == LOW && now == HIGH) { /* rising edge: LOW->HIGH */
      Serial.println("[Action] automation done");
      active_ = false;
      digitalWrite(TX_PIN, LOW); // reset output
      if (doneCb_) {
        DoneCb cb = doneCb_;  // copy in case cb restarts us
        doneCb_ = nullptr;
        cb();  // notify caller exactly once
      }
    }
    last = now;
  }

private:
  DoneCb doneCb_ = nullptr;
  bool   active_ = false;
  bool last = HIGH;
  uint8_t inputMode;
};

#endif