# atm_automation
ATM Automation Sketches

## RFID Scanner

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

The `setup()` will be called from the main `setup()` routine.

The `run(callback)` will be called to start the automation.  A `void (*)` function can be passed as a "done callback", which should be called when the automation completes.

#### DigitalSignalAutomation

Enable by uncommenting the following lines in `config.h`:

```c++
#include "DigitalSignalAutomation.h"
DigitalSignalAutomation automation;
```

Pinout:
* Arduino pin 5 - TX, defaults to `LOW`
* Arduino pin 4 - RX, defaults to `INPUT_PULLUP`

The `DigitalSignalAutomation` has the following functionality:

* When automation is triggered, writes `HIGH` to TX pin
* Then waits to detect a rising edge `LOW` -> `HIGH` on RX pin
  * When detected, executes callback passed to `run()` 

#### SoundAutomation

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

##### MP3 Memory Card Tricks

The DY-* modules are very particular about the names of files, and the existence of additional files on the memory card.

If using MacOS, it will add a bunch of Spotlight-related files that will mess stuff up.

###### Cleaning up spotlight files

```bash
> sudo mdutil -d /Volumes/NO\ NAME
> sudo mdutil -X /Volumes/NO\ NAME
> find /Volumes/NO\ NAME -type f -name '.*' -delete
> ls -al /Volumes/NO\ NAME
```

You should see only your .mp3 files (and maybe a single `.fseventsd` file).

###### Cleanly copying files

Create a directory (using Spotlight is fine) to keep your .mp3 files in.  After adding them, and naming them appropriately, run the following (assumes you are keeping things in `~/Documents/mp3s`:

```bash
> cd ~/Documents/mp3s
> cp -v `ls -1 *.mp3|sort` /Volumes/NO\ NAME
> diskutil umount /Volumes/NO\ NAME
```
