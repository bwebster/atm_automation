/*  DigitalSignalAutomation.h
    ──────────────────────────────────────────────────────────────
    • Drives pin 8 HIGH,
    • spins until pin 7 reads HIGH,
    • then invokes the callback.
    • Blocking – keeps the code base minimal and avoids timers /
      interrupts.  Suitable when the handshake is expected to be quick.
*/

#ifndef DIGITAL_SIGNAL_AUTOMATION_H
#define DIGITAL_SIGNAL_AUTOMATION_H

#include "Automation.h"

const uint8_t TX_PIN = 5;
const uint8_t RX_PIN = 4;

class DigitalSignalAutomation : public Automation {
public:
  /* one-time hardware setup */
  void setup() override {
    Serial.println("Setting up digital signal automation");
    pinMode(TX_PIN, OUTPUT);
    pinMode(RX_PIN, INPUT);
    digitalWrite(TX_PIN, LOW);
    digitalWrite(RX_PIN, LOW);
  }

  void run(DoneCb cb) override {
    digitalWrite(TX_PIN, HIGH);         // signal out
    doneCb_ = cb;
    active_ = true;
  }

  /* poll peer in loop() */
  void update() override {
    if (!active_) return;

    digitalWrite(TX_PIN, LOW);

    if (digitalRead(RX_PIN) == HIGH) {  // other side done
      Serial.println("Automation - RX PIN HIGH");
      active_ = false;
      if (doneCb_) {
        DoneCb cb = doneCb_;            // copy in case cb restarts us
        doneCb_ = nullptr;
        cb();                           // notify caller exactly once
      }
      // digitalWrite(TX_PIN, LOW);        // reset output
    }
  }

private:
  DoneCb doneCb_ = nullptr;
  bool   active_ = false;
};

#endif