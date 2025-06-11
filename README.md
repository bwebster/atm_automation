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

### Customizing Automation

The code has a couple different automations that can be enabled, which implement a pretty basic interface

- `setup()`
- `run(callback)`

The `setup()` will be called from the main `setup()` routine.

The `run(callback)` will be called to start the automation.  A `void (*)` function can be passed as a "done callback", which should be called when the automation completes.

#### DigitalSignalAutomation

Implemented by `rfid_scanner/DigitalSignalAutomation.h`.

#### SoundAutomation

Implemented by `rfid_scanner/SoundAutomation.h`.

## MP3 Memory Card Tricks

The DY-* modules are very particular about the names of files, and the existence of additional files on the memory card.

If using MacOS, it will add a bunch of Spotlight-related files that will mess stuff up.

**Cleaning up spotlight files**

```bash
> sudo mdutil -d /Volumes/NO\ NAME
> sudo mdutil -X /Volumes/NO\ NAME
> find /Volumes/NO\ NAME -type f -name '.*' -delete
> ls -al /Volumes/NO\ NAME
```

You should see only your .mp3 files (and maybe a single `.fseventsd` file).

**Cleanly copying files**

Create a directory (using Spotlight is fine) to keep you .mp3 files in.  After adding them, and naming them appropriately, run the following:

```bash
> cd ~/Documents/mp3s
> cp -v `ls -1 *.mp3|sort` /Volumes/NO\ NAME
> diskutil umount /Volumes/NO\ NAME
```
