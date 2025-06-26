#ifndef CONFIG_H
#define CONFIG_H

// ----------
// Automation
// ----------
#include "NoAutomation.h"
NoAutomation automation;

// DigitalSignalAutomation will send a HIGH signal to start automation, and wait for a HIGH signal to indicate it's done.
// RX pin defaults to INPUT, but can be changed by passing a different mode to constructor, e.g. DigitalSignalAutomation automation(INPUT_PULLUP).
// #include "DigitalSignalAutomation.h"
// DigitalSignalAutomation automation;

// DigitalSignalLowAutomation will send a HIGH signal to start automation, want wait for a LOW signal to indicate it's done.
// RX pin defaults to INPUT_PULLUP.
#include "DigitalSignalLowAutomation.h"
DigitalSignalLowAutomation automation;

// #include "SoundAutomation.h"
// SoundAutomation automation;

// #include "WledAutomation.h"
// WledAutomation automation;

// #include "WledSoundAutomation.h"
// WledSoundAutomation automation;


// --------
// Scanning
// --------
// Max time allowed to process a scan, e.g. send it to the server.
// If timeout is hit, will move on to next state.
#define SCAN_TIMEOUT_MS 5000

// Max time allowed for automation to complete.  
// If timeout is hit, will move back to ready state.
#define AUTOMATION_TIMEOUT_MS 15000

// Time to wait before clearning scan history.
// Set to 0 to not automatically clear.  
// If set to 0, and RECENT_SCAN_HISTORY_SIZE > 0, you will not be able to scan a tag multiple times in a row.
#define CLEAR_HISTORY_AFTER_MS 30'000

// Number of tags to keep in the history list. If a tag is in the list, it cannot be rescanned.
#define RECENT_SCAN_HISTORY_SIZE 1

// ---------------
// General Config
// ---------------

// Location number to send after successful scan
#define LOCATION 2

// Delay between health check calls
#define HEALTH_CHECK_INTERVAL_MS 1000 * 60

// Configure the IP address and port for the server software.
// const char *server = "192.168.5.229";
// const int port = 3000;
const char* server = "atm-clv-37eca624ed8b.herokuapp.com";
const int port = 80;

// Add wifi credentials, to be tried in order until a successful connection.
// For local development you can define credentials[] in a secrets.h file to add
// you home network.
//
// Sample secrets.h file:
//
//   #ifndef SECRETS_H
//   #define SECRETS_H
//   #define CREDENTIALS
//   static const WifiCredential credentials[] = {
//     {"MyNetwork", "MyPassword"},
//     {"Life.Church", nullptr},  // open network
//   };
//   #endif
//
// Then simply uncomment the #include "secrets.h" line below.
// #include "secrets.h"

// Default configuration if there is no secrets.h file
#ifndef CREDENTIALS
static const WifiCredential credentials[] = {
  { "Life.Church", nullptr },  // open network
};
#endif

#endif