#ifndef NO_AUTOMATION_H
#define NO_AUTOMATION_H

#include "Automation.h"

// Used when no automation is needed.
class NoAutomation : public Automation {
public:
  void setup() override {
    // No-op
  }

  void run(DoneCb cb) override {
    Serial.println("[Action] no automation started");
    doneCb_ = cb;  // Save the callback to be called in update()
    active_ = true;
    runAt_ = millis();
  }

  void update() override {
    if (!active_) {
      return;
    }

    if (runAt_ > 0) {
      int elapsed = millis() - runAt_;
      if (elapsed < 3000) {
          return;
      }
    }

    Serial.println("[Action] no automation done");
    if (doneCb_) {
      DoneCb cb = doneCb_;
      doneCb_ = nullptr;
      active_ = false;
      cb();  // Call exactly once
    }
  }

  void cancel() override {
    doneCb_ = nullptr;
    active_ = false;
    runAt_ = 0;
  }

private:
  DoneCb doneCb_ = nullptr;
  bool active_ = false;
  unsigned long runAt_;
};

#endif
