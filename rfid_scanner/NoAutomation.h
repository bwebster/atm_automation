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
    doneCb_ = cb;  // Save the callback to be called in update()
    active_ = true;
  }

  void update() override {
    if (active_ && doneCb_) {
      DoneCb cb = doneCb_;
      doneCb_ = nullptr;
      active_ = false;
      cb();  // Call exactly once
    }
  }

  void cancel() override {
    doneCb_ = nullptr;
    active_ = false;
  }

private:
  DoneCb doneCb_ = nullptr;
  bool active_ = false;
};

#endif
