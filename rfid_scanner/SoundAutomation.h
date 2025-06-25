#ifndef DIGITAL_SIGNAL_AUTOMATION_H
#define DIGITAL_SIGNAL_AUTOMATION_H

#include "Automation.h"
#include <SoftwareSerial.h>
#include <DYPlayerArduino.h>  // External: https://github.com/SnijderC/dyplayer (download zip and add manually)

#define TX_PIN 5
#define RX_PIN 4
#define BUSY_PIN 3
#define BAUD 9600

class SoundAutomation : public Automation {
public:
  SoundAutomation() 
    : mp3Serial(RX_PIN, TX_PIN), player(&mp3Serial) {};

  void setup() override {
    Serial.println("Setting up sound automation");

    pinMode(BUSY_PIN, INPUT_PULLUP);  // BUSY from board; HIGH by default, LOW when playing

    mp3Serial.begin(BAUD);
    delay(800);
    player.begin();

    numTracks = getNumberTracks();
    Serial.print("Found number of tracks: ");
    Serial.println(numTracks);

    player.setVolume(30);  // 0...30
  }

  void run(DoneCb cb) override {
    Serial.print("[Action] Playing track ");
    Serial.println(track);
    player.playSpecified(track);
    doneCb_ = cb;
    active_ = true;
    track += 1;
    if (track > numTracks) {
      track = 1;
    }
  }

  void update() override {
    if (!active_) return;

    bool now = digitalRead(BUSY_PIN);
    if (last == LOW && now == HIGH) { /* rising edge: LOW->HIGH */
      Serial.println("Track finished");
      player.stop();
      active_ = false;
      if (doneCb_) {
        DoneCb cb = doneCb_;  // copy in case cb restarts us
        doneCb_ = nullptr;
        cb();  // notify caller exactly once
      }
    }
    last = now;
  }

  void cancel() override {
    if (!active_) return;

    doneCb_ = nullptr;
  }

private:
  DoneCb doneCb_ = nullptr;
  bool active_ = false;
  SoftwareSerial mp3Serial;
  DY::Player player;
  bool last = LOW;
  int track = 1;
  int numTracks = 1;

  int getNumberTracks() {
    player.setVolume(0);
    delay(100);

    int count = 1;
    for (; count < 255; count++) {
      player.playSpecified(count);
      delay(500);
      if (digitalRead(BUSY_PIN) == HIGH) {
        player.stop();
        count -= 1;
        break;
      }

      player.stop();
      delay(500);
    }
    return count;
  }
};

#endif