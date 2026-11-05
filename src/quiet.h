// Quiet hours (outside unit). Between `from` and `to` (local time, minutes
// after midnight, may cross midnight) both units play quieter: the outside
// speaker at `outPct`, the inside chime at `inPct` of normal (0 = silent,
// 100 = normal; the pads still light). Settings live in flash and are edited
// from the pad web page. Local time comes from the internet (NTP) once WiFi
// is up and keeps running on the chip's clock after that; until the first
// sync the clock is unknown and quiet hours do not apply.
#pragma once

#include <cstddef>
#include <cstdint>

namespace quiet {

struct Settings {
  bool     on;
  uint16_t from;    // minutes after midnight, 0..1439
  uint16_t to;
  uint8_t  outPct;  // 0..100
  uint8_t  inPct;
};

void begin();                          // load settings, start the clock
void loop(uint32_t nowMs);             // re-evaluate once a second
const Settings& settings();
bool set(const Settings& s);           // validate + save; false if out of range
bool clockKnown();
bool active();                         // quiet hours apply right now
int  nowMinutes();                     // -1 if the clock is unknown
int  outsidePct();                     // 100 when not active
int  insidePct();
// JSON for the web page: settings + clock + active.
void toJson(char* buf, size_t len);

}  // namespace quiet
