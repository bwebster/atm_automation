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
#include <SoftwareSerial.h>
#include <DYPlayerArduino.h> // External: https://github.com/SnijderC/dyplayer (download zip and add manually)

constexpr uint8_t TX_PIN = 5;
constexpr uint8_t RX_PIN = 4;
constexpr uint8_t BUSY_PIN = 3;
constexpr uint8_t BAUD = 9600;

// SoftwareSerial mp3Serial(RX_PIN, TX_PIN);
// DY::Player     player(&mp3Serial);

class SoundAutomation : public Automation {
public:
  SoundAutomation()
  : mp3Serial(RX_PIN, TX_PIN),
    player(&mp3Serial) {}

  /* one-time hardware setup */
  void setup() override {
    Serial.println("Setting up sound automation");

    pinMode(BUSY_PIN, INPUT_PULLUP);

    mp3Serial.begin(BAUD);
    delay(1000);
    player.begin();
    player.setVolume(25);
  }

  void run(DoneCb cb) override {    
    Serial.println("Playing track 1");
    player.begin();
    player.setVolume(25);
    player.playSpecified(1);
    doneCb_ = cb;
    active_ = true;
  }

  void update() override {
    if (!active_) return;

    static bool last = HIGH;
    bool now = digitalRead(BUSY_PIN);

    if (last == HIGH && now == LOW) {
      Serial.println(F("Track started"));
    } 
    if (last == LOW  && now == HIGH) {
      Serial.println(F("Track finished"));
      active_ = false;
      if (doneCb_) {
        DoneCb cb = doneCb_;            // copy in case cb restarts us
        doneCb_ = nullptr;
        cb();                           // notify caller exactly once
      }
      player.stop();
    }

    last = now;
  }

private:
  SoftwareSerial mp3Serial;
  DY::Player     player;
  DoneCb doneCb_ = nullptr;
  bool   active_ = false;
};

#endif