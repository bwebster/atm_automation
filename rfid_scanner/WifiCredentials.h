#ifndef WIFI_CREDENTIALS_H
#define WIFI_CREDENTIALS_H

struct WifiCredential {
  const char* ssid;
  const char* password;  // Can be NULL or empty string
};

#endif