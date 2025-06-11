#ifndef DIGITAL_SIGNAL_AUTOMATION_H
#define DIGITAL_SIGNAL_AUTOMATION_H

#include "Automation.h"

const uint8_t TX_PIN = 5; // TX to sound module; HIGH to start sound
const uint8_t RX_PIN = 4; // RX from sound module; HIGH by default, LOW when playing

class DigitalSignalAutomation : public Automation {
public:
  /* one-time hardware setup */
  void setup() override {
    Serial.println("Setting up digital signal automation");
    pinMode(TX_PIN, OUTPUT);
    digitalWrite(TX_PIN, LOW);
    pinMode(RX_PIN, INPUT_PULLUP);
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

    if (digitalRead(RX_PIN) == LOW) { // playing
      return;
    }

    Serial.println("[Action] automation is done via RX");
    active_ = false;
    if (doneCb_) {
      DoneCb cb = doneCb_;            // copy in case cb restarts us
      doneCb_ = nullptr;
      cb();                           // notify caller exactly once
    }
    
    Serial.println("[Action] set automation TX to LOW");
    digitalWrite(TX_PIN, LOW);        // reset output
  }

private:
  DoneCb doneCb_ = nullptr;
  bool   active_ = false;
};

#endif