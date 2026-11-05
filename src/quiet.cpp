#include "quiet.h"

#if defined(UNIT_OUTSIDE) && !defined(PIXEL_TEST) && !defined(LINK_TEST)

#include <Arduino.h>
#include <Preferences.h>

#include <ctime>

namespace quiet {
namespace {

// Israel: UTC+2, summer time from the Friday before the last Sunday of March
// to the last Sunday of October.
constexpr const char* kTz = "IST-2IDT,M3.4.4/26,M10.5.0";
constexpr const char* kNvs = "quiet";

Settings cfg = {false, 22 * 60, 7 * 60, 25, 50};
bool isActive = false;
uint32_t lastEval = 0;
Preferences prefs;

bool valid(const Settings& s) {
  return s.from < 1440 && s.to < 1440 && s.outPct <= 100 && s.inPct <= 100;
}

bool inWindow(int now, int from, int to) {
  if (from == to) return false;
  if (from < to) return now >= from && now < to;
  return now >= from || now < to;   // crosses midnight
}

void evaluate() {
  const int now = nowMinutes();
  const bool was = isActive;
  isActive = cfg.on && now >= 0 && inWindow(now, cfg.from, cfg.to);
  if (isActive != was) {
    Serial.printf("quiet:    %s (%02d:%02d)\n", isActive ? "ON, playing quieter" : "off, normal volume",
                  now / 60, now % 60);
  }
}

}  // namespace

void begin() {
  prefs.begin(kNvs, false);
  Settings s;
  s.on     = prefs.getBool("on", cfg.on);
  s.from   = prefs.getUShort("from", cfg.from);
  s.to     = prefs.getUShort("to", cfg.to);
  s.outPct = prefs.getUChar("out", cfg.outPct);
  s.inPct  = prefs.getUChar("in", cfg.inPct);
  if (valid(s)) cfg = s;
  // SNTP starts polling as soon as WiFi is up; nothing to do here if it is not.
  configTzTime(kTz, "pool.ntp.org", "time.google.com");
  Serial.printf("Quiet:    %s, %02u:%02u-%02u:%02u, outside %u%%, inside %u%% (clock from NTP)\n",
                cfg.on ? "on" : "off", cfg.from / 60, cfg.from % 60, cfg.to / 60, cfg.to % 60,
                cfg.outPct, cfg.inPct);
}

void loop(uint32_t nowMs) {
  if (nowMs - lastEval < 1000) return;
  lastEval = nowMs;
  evaluate();
}

const Settings& settings() { return cfg; }

bool set(const Settings& s) {
  if (!valid(s)) return false;
  cfg = s;
  prefs.putBool("on", cfg.on);
  prefs.putUShort("from", cfg.from);
  prefs.putUShort("to", cfg.to);
  prefs.putUChar("out", cfg.outPct);
  prefs.putUChar("in", cfg.inPct);
  evaluate();
  Serial.printf("quiet:    saved %s, %02u:%02u-%02u:%02u, outside %u%%, inside %u%%\n",
                cfg.on ? "on" : "off", cfg.from / 60, cfg.from % 60, cfg.to / 60, cfg.to % 60,
                cfg.outPct, cfg.inPct);
  return true;
}

bool clockKnown() {
  // Before the first sync the clock reads 1970.
  return time(nullptr) > 1700000000;
}

int nowMinutes() {
  if (!clockKnown()) return -1;
  const time_t t = time(nullptr);
  struct tm lt;
  localtime_r(&t, &lt);
  return lt.tm_hour * 60 + lt.tm_min;
}

bool active() { return isActive; }
int outsidePct() { return isActive ? cfg.outPct : 100; }
int insidePct() { return isActive ? cfg.inPct : 100; }

void toJson(char* buf, size_t len) {
  snprintf(buf, len,
           "{\"on\":%s,\"from\":%u,\"to\":%u,\"out\":%u,\"in\":%u,\"now\":%d,\"active\":%s}\n",
           cfg.on ? "true" : "false", cfg.from, cfg.to, cfg.outPct, cfg.inPct, nowMinutes(),
           isActive ? "true" : "false");
}

}  // namespace quiet

#endif
