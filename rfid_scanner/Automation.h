#ifndef AUTOMATION_H
#define AUTOMATION_H
#include <Arduino.h>

/* =====================================================================
 *  Automation.h — minimal non‑blocking automation interface
 *
 *  Derive from Automation when you need a tiny state‑machine that…
 *    • does one‑time hardware setup (setup())
 *    • starts on demand (run())
 *    • progresses without blocking (update())
 *    • calls a user‑supplied DoneCb exactly once when finished
 *
 *  No dynamic allocation, no <functional>; the callback is a plain
 *  C‑style pointer so it works on every Arduino target.
 * ===================================================================== */

class Automation {
public:
  using DoneCb = void (*)();        // callback type: void fn()

  /* One‑time initialization — called from the sketch’s setup().
     Override if your automation needs pinMode(), etc.        */
  virtual void setup() {}

  /* Kick off the automation.  Implementation saves cb and returns
     immediately so loop() can keep running.                  */
  virtual void run(DoneCb cb) = 0;

  /* Advance the automation; should be cheap and non‑blocking.
     Call the saved cb once the work is complete.             */
  virtual void update() = 0;

  virtual ~Automation() = default;
};
#endif
