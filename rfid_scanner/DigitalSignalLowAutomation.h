#ifndef DIGITAL_SIGNAL_LOW_AUTOMATION_H
#define DIGITAL_SIGNAL_LOW_AUTOMATION_H

#include "Automation.h"

const uint8_t TX_PIN = 5;  // TX to module; HIGH to start
const uint8_t RX_PIN = 4;  // RX from module; HIGH when done

class DigitalSignalLowAutomation : public Automation {
public:
  DigitalSignalLowAutomation() {}

  /* one-time hardware setup */
  void setup() override {
    Serial.println("Setting up digital signal low automation");
    pinMode(TX_PIN, OUTPUT);
    pinMode(RX_PIN, INPUT_PULLUP);
    digitalWrite(TX_PIN, HIGH);
  }

  void run(DoneCb cb) override {
    Serial.println("[Action] reset TX to LOW before HIGH to create rising edge");

    // Generate a clean LOW-to-HIGH edge
    digitalWrite(TX_PIN, LOW);   // force LOW
    // delay(10);                   // brief delay to ensure LOW state is latched
    // digitalWrite(TX_PIN, HIGH);  // rising edge now happens
    last = HIGH;
    doneCb_ = cb;
    active_ = true;
  }

  /* poll peer in loop() */
  void update() override {
    if (!active_) return;

    bool now = digitalRead(RX_PIN);
    if (last == HIGH && now == LOW) {
      Serial.println("[Action] automation done - falling edge detected");
      active_ = false;
      digitalWrite(TX_PIN, HIGH);  // reset signal so it’s ready for next run
      if (doneCb_) {
        DoneCb cb = doneCb_;
        doneCb_ = nullptr;
        cb();  // notify caller
      }
    }
    last = now;
  }

  void cancel() override {
    if (!active_) return;

    Serial.println("[Cancel] automation was cancelled");
    active_ = false;
    doneCb_ = nullptr;
    digitalWrite(TX_PIN, HIGH);  // reset signal so it’s ready for next run
  }

private:
  DoneCb doneCb_ = nullptr;
  bool active_ = false;
  bool last = LOW;  // updated default to match LOW starting state
};

#endif
