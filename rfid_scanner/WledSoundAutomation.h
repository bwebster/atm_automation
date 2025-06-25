#ifndef WLED_SOUND_AUTOMATION_H
#define WLED_SOUND_AUTOMATION_H

#include "Automation.h"
#include <SoftwareSerial.h>
#include <SerialTransfer.h>  // External: https://github.com/PowerBroker2/SerialTransfer v3.1.4+

#define TX_PIN 5
#define RX_PIN 4
#define BAUD 9600

const int START = 0;
const int STOP = 1;
const int DONE = 2;

struct __attribute__((__packed__)) Payload {
  int cmd;  
};

class WledSoundAutomation : public Automation {
public:
  WledSoundAutomation()
    : cmdSerial(RX_PIN, TX_PIN) {}

  void setup() override {
    Serial.println("Setting up WLED sound automation");

    cmdSerial.begin(BAUD);        // Software serial for ESP32 comms
    myTransfer.begin(cmdSerial, false);  // Must be called after begin()
  }

  void run(DoneCb cb) override {
    doneCb_ = cb;
    active_ = true;

    Serial.println("Sending command: START");
    Payload command = { START };
    myTransfer.txObj(command);
    myTransfer.sendData(sizeof(command));
    // delay(100);
    lastDebug = millis();
  }

  void update() override {
    if (!active_) return;

    if (myTransfer.available()) {
      Payload data;
      myTransfer.rxObj(data);
      Serial.print("Cmd: ");
      Serial.print(data.cmd);

      if (data.cmd == DONE) {
        active_ = false;
        if (doneCb_) {
          DoneCb cb = doneCb_;  // copy in case cb restarts us
          doneCb_ = nullptr;
          cb();  // notify caller exactly once
        }
      }
    } else if (millis() - lastDebug > 1000) {
      Serial.print(myTransfer.status);
      lastDebug = millis();
    }
  }

  void cancel() override {
    if (!active_) return;

    Serial.println("Sending command: STOP");
    Payload data = { STOP };
    myTransfer.txObj(data);
    myTransfer.sendData(sizeof(data));
    delay(100);

    doneCb_ = nullptr;
  }

private:
  DoneCb doneCb_ = nullptr;
  bool active_ = false;

  SoftwareSerial cmdSerial;
  SerialTransfer myTransfer;

  unsigned long lastDebug;

  bool last = LOW;
};

#endif