#include "notify.h"

#if defined(UNIT_OUTSIDE) && !defined(PIXEL_TEST) && !defined(LINK_TEST)

#include <Arduino.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <cstring>

namespace notify {
namespace {

constexpr const char* kNvs = "notify";

struct Msg { char title[48]; char body[96]; char tags[32]; };

Settings cfg = {false, "https://ntfy.sh", ""};
Preferences prefs;
QueueHandle_t queue;
volatile bool okLast = true;

bool validTopic(const char* t) {
  const size_t n = strlen(t);
  if (n == 0 || n > 63) return false;
  for (size_t i = 0; i < n; ++i) {
    const char c = t[i];
    if (!(isalnum(c) || c == '_' || c == '-')) return false;
  }
  return true;
}

bool valid(const Settings& s) {
  const size_t n = strlen(s.server);
  if (n < 8 || n > 63) return false;
  if (strncmp(s.server, "http://", 7) != 0 && strncmp(s.server, "https://", 8) != 0) return false;
  return !s.on || validTopic(s.topic);
}

bool post(const Msg& m) {
  if (WiFi.status() != WL_CONNECTED) { Serial.println("notify:   no WiFi, dropped"); return false; }
  String url = String(cfg.server) + "/" + cfg.topic;
  HTTPClient http;
  WiFiClientSecure tls;
  bool begun;
  if (url.startsWith("https://")) {
    tls.setInsecure();   // a push message; the topic is the secret, not the transport
    begun = http.begin(tls, url);
  } else {
    begun = http.begin(url);
  }
  if (!begun) { Serial.println("notify:   bad URL"); return false; }
  http.setTimeout(8000);
  http.addHeader("Title", m.title);
  http.addHeader("Tags", m.tags);
  http.addHeader("Priority", "high");
  const int code = http.POST(String(m.body));
  http.end();
  Serial.printf("notify:   %s -> %d\n", m.body, code);
  return code >= 200 && code < 300;
}

void task(void*) {
  Msg m;
  for (;;) {
    if (xQueueReceive(queue, &m, portMAX_DELAY) == pdTRUE) okLast = post(m);
  }
}

}  // namespace

void begin() {
  prefs.begin(kNvs, false);
  Settings s = cfg;
  s.on = prefs.getBool("on", false);
  prefs.getString("server", s.server, sizeof s.server);
  prefs.getString("topic", s.topic, sizeof s.topic);
  if (!s.server[0]) strcpy(s.server, "https://ntfy.sh");
  if (valid(s)) cfg = s;
  queue = xQueueCreate(4, sizeof(Msg));
  // Core 0 with WiFi; 8 KB stack for TLS.
  xTaskCreatePinnedToCore(task, "notify", 8192, nullptr, 1, nullptr, 0);
  Serial.printf("Notify:   %s, %s/%s\n", cfg.on ? "on" : "off", cfg.server,
                cfg.topic[0] ? cfg.topic : "(no topic)");
}

const Settings& settings() { return cfg; }

bool set(const Settings& s) {
  if (!valid(s)) return false;
  cfg = s;
  prefs.putBool("on", cfg.on);
  prefs.putString("server", cfg.server);
  prefs.putString("topic", cfg.topic);
  Serial.printf("notify:   saved %s, %s/%s\n", cfg.on ? "on" : "off", cfg.server, cfg.topic);
  return true;
}

bool send(const char* title, const char* message, const char* tags) {
  if (!cfg.on || !validTopic(cfg.topic)) return false;
  Msg m;
  strlcpy(m.title, title, sizeof m.title);
  strlcpy(m.body, message, sizeof m.body);
  strlcpy(m.tags, tags, sizeof m.tags);
  return xQueueSend(queue, &m, 0) == pdTRUE;
}

bool lastOk() { return okLast; }

void toJson(char* buf, size_t len) {
  snprintf(buf, len, "{\"on\":%s,\"server\":\"%s\",\"topic\":\"%s\",\"ok\":%s}\n",
           cfg.on ? "true" : "false", cfg.server, cfg.topic, okLast ? "true" : "false");
}

}  // namespace notify

#endif
