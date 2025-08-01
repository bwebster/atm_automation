# atm_automation
ATM Automation Sketches

## RFID Scanner

<img width="848" alt="Screenshot 2025-06-11 at 4 27 07 PM" src="https://github.com/user-attachments/assets/234d8a2c-268a-47dd-a0a1-ec826841bb6b" />

The `rfid_scanner/rfid_scanner.ino` sketch will be installed onto each Arduino R4 UNO WiFi.  

The general behavior is:

- Setup: connect to wifi, initialized RFID scanner
- Loop:
  - Watch for scan
  - Turn on LEDs
  - Submit tag information to software via HTTP
  - Enable automation
  - Wait for automation to complete, or until we hit a configurable timeout

### Customizing Behavior

All customization options can be found in `config.h`, along with comments.

### Customizing Automation

The code has a couple different automations that can be enabled, which implement a pretty basic interface

- `setup()`
- `run(callback)`
- `cancel()`

The `setup()` will be called from the main `setup()` routine.

The `run(callback)` will be called to start the automation.  A `void (*)` function can be passed as a "done callback", which should be called when the automation completes.

The `cancel()` function can be called at any time to cancel an automation.  The `cancel()` implementation should reset the state so that the automation is ready to be triggered again.

#### NoAutomation

Enable by uncommenting the following lines in `config.h`:

```c++
#include "NoAutomation.h"
NoAutomation automation;
```

This should be used when no automation is needed.

#### DigitalSignalAutomation

Enable by uncommenting the following lines in `config.h`:

```c++
#include "DigitalSignalAutomation.h"
DigitalSignalAutomation automation;
```

Pinout:
* Arduino pin 5 - TX, defaults to `LOW`
* Arduino pin 4 - RX, defaults to `INPUT_PULLDOWN`

You can override the RX pin default by passing in a different pin mode, e.g.

```c++
#include "DigitalSignalAutomation.h"
DigitalSignalAutomation automation(INPUT_PULLUP);
```

The `DigitalSignalAutomation` has the following functionality:

* When automation is triggered, writes `HIGH` to TX pin
* Then waits to detect a rising edge `LOW` -> `HIGH` on RX pin
  * When detected, executes callback passed to `run()`
* Can be cancelled by calling `cancel()`
  
#### DigitalSignalLowAutomation

Enable by uncommenting the following lines in `config.h`:

```c++
#include "DigitalSignalLowAutomation.h"
DigitalSignalLowAutomation automation;
```

Pinout:
* Arduino pin 5 - TX, defaults to `LOW`
* Arduino pin 4 - RX, defaults to `INPUT_PULLUP`

The `DigitalSignalLowAutomation` has the following functionality:

* When automation is triggered, writes `HIGH` to TX pin
* Then waits to detect a falling edge `HIGH` -> `LOW` on RX pin
  * When detected, executes callback passed to `run()`
* Can be cancelled by calling `cancel()`

#### SoundAutomation

<img width="734" alt="Screenshot 2025-06-11 at 4 28 45 PM" src="https://github.com/user-attachments/assets/7986789c-4775-4b96-b55d-b7704ba92f2e" />

Enable by uncommenting the following lines in `config.h`:

```c++
#include "SoundAutomation.h"
SoundAutomation automation;
```

Pinout:
* Arduino pin 5 - Serial TX
* Arduino pin 4 - Serial RX
* Arduino pin 3 - BUSY pin, defaults to `INPUT_PULLUP`

The `SoundAutomation` has the following functionality:

* Sets `track` to 1
* When automation is triggered, writes serials commands to trigger track `track`
  * Increments `track`
* Then waits to detect a rising edge `LOW` -> `HIGH` on BUSY pin
  * When detected, executes callback passed to `run()` 
* Can be cancelled by calling `cancel()`

##### MP3 Memory Card Tricks

The DY-* modules are very particular about the names of files, and the existence of additional files on the memory card.

If using MacOS, it will add a bunch of Spotlight-related files that will mess stuff up.

How to get mp3 files on the memory card:

1. Download your mp3 files and place them in `Documents/mp3s`, using the naming convention `00001.mp3`, `00002.mp3`, etc.
2. Insert your memory card
3. Remove extra files (note that it's okay to have a `.fseventsd` present)
  ```bash
  sudo mdutil -d /Volumes/NO\ NAME
  sudo mdutil -X /Volumes/NO\ NAME
  find /Volumes/NO\ NAME -type f -name '.*' -delete
  ls -al /Volumes/NO\ NAME
  ```
4. Copy over mp3s using terminal:
  ```bash
  cd ~/Documents/mp3s
  cp -v `ls -1 mp3s/*.mp3|sort` /Volumes/NO\ NAME
  ```
5. Unmount the memory card using terminal:
  ```bash
  diskutil umount /Volumes/NO\ NAME
  ```

#### WledAutomation

<img width="881" alt="Screenshot 2025-06-20 at 8 53 47 AM" src="https://github.com/user-attachments/assets/5d39cba7-121f-4675-a85d-2324b70661e6" />

Enable by uncommenting the following lines in `config.h`:

```c++
#include "WledAutomation.h"
WledAutomation automation;
```

Pinout:
* Arduino pin 5 - Serial TX
* Arduino pin 4 - Serial RX

The `WledAutomation` has the following functionality:

* Sets `preset=1`
* When automation is triggered, turns on LEDs to preset `preset`
  * Increments `preset`
* Waits a configurable amount of time
* Turns off LEDs
* Can be cancelled by calling `cancel()`
