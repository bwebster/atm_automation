# atm_automation
ATM Automation Sketches


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
