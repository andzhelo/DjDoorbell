// Template for WiFi credentials. Copy to include/secrets.h (gitignored) and
// fill in. This repo is public — secrets.h must never be committed.
// Without secrets.h both units run ESP-NOW only on channel 1: no OTA.
#pragma once

#define WIFI_SSID     "your-ssid"       // a 2.4 GHz network: the ESP32 has no 5 GHz radio
#define WIFI_PASSWORD "your-password"
// Protects over-the-air updates. Upload with:
//   DJ_OTA_PASSWORD=... pio run -e inside_ota -t upload
#define OTA_PASSWORD  "choose-one"
