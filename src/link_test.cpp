// Phase 3 link test — the GATE. pio run -e linktest_outside / linktest_inside
//
// Outside (sender, at the door): 50 packets/s, alternating
//   RAW  broadcast, no MAC retries  -> raw radio loss through the door
//   UNI  unicast with MAC retries   -> what the real doorbell will see
// and reports its own view (unicast delivered/failed, echoes, round trip)
// inside every packet. Onboard RGB LED: green = echoes arriving, red = none.
//
// Inside (receiver, on the laptop): counts received/lost per kind from the
// sequence numbers, RSSI and the longest silence, echoes every packet back so
// the reverse direction and round trip are measured too, prints a line every
// 10 s and a PASS/FAIL summary after 10 minutes.
//
// millis()/micros() are fine here: this measures a radio, not music.
#ifdef LINK_TEST

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#ifndef LINK_CHANNEL
#define LINK_CHANNEL 1   // must match the router channel once the inside unit joins WiFi
#endif

constexpr uint32_t kMagic       = 0xD1D0B311;
constexpr uint32_t kPeriodMs    = 20;          // 50 packets/s total
constexpr uint32_t kReportMs    = 10000;
constexpr uint32_t kTestMs      = 10 * 60 * 1000;
constexpr uint8_t  kBroadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

enum Kind : uint8_t { RAW = 0, UNI = 1, ECHO = 2 };

struct __attribute__((packed)) Packet {
  uint32_t magic;
  uint8_t  kind;
  uint32_t seq;        // per kind
  uint32_t sentUs;     // sender's micros(), echoed back for round trip
  // sender's view, carried to the inside log:
  uint32_t uniSent, uniOk, uniFail, echoRx, rttAvgUs, rttMaxUs;
};

static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

static void radioInit() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_channel(LINK_CHANNEL, WIFI_SECOND_CHAN_NONE);
  if (esp_now_init() != ESP_OK) {
    Serial.println("FATAL: esp_now_init failed");
    for (;;) delay(1000);
  }
  Serial.printf("MAC %s, channel %d\n", WiFi.macAddress().c_str(), LINK_CHANNEL);
}

static void addPeer(const uint8_t* mac) {
  if (esp_now_is_peer_exist(mac)) return;
  esp_now_peer_info_t p = {};
  memcpy(p.peer_addr, mac, 6);
  p.channel = LINK_CHANNEL;
  p.ifidx = WIFI_IF_STA;
  esp_now_add_peer(&p);
}

// ============================================================================
#if defined(UNIT_OUTSIDE)

static uint8_t  insideMac[6];
static volatile bool haveInside = false, needPeer = false;
static uint32_t seq[2];
static uint32_t uniSent, uniOk, uniFail, echoRx;
static uint64_t rttSumUs;
static uint32_t rttMaxUs, lastEchoMs;

static void onSent(const esp_now_send_info_t* info, esp_now_send_status_t status) {
  if (memcmp(info->des_addr, kBroadcast, 6) == 0) return;   // RAW: no ACK exists
  portENTER_CRITICAL(&mux);
  (status == ESP_NOW_SEND_SUCCESS ? uniOk : uniFail)++;
  portEXIT_CRITICAL(&mux);
}

static void onRecv(const esp_now_recv_info_t* info, const uint8_t* data, int len) {
  if (len != sizeof(Packet)) return;
  Packet p;
  memcpy(&p, data, sizeof p);
  if (p.magic != kMagic || p.kind != ECHO) return;
  const uint32_t rtt = micros() - p.sentUs;
  portENTER_CRITICAL(&mux);
  echoRx++;
  rttSumUs += rtt;
  if (rtt > rttMaxUs) rttMaxUs = rtt;
  lastEchoMs = millis();
  if (!haveInside) { memcpy(insideMac, info->src_addr, 6); needPeer = true; }
  portEXIT_CRITICAL(&mux);
}

static void led(uint8_t r, uint8_t g) {
#ifdef RGB_BUILTIN
  rgbLedWrite(RGB_BUILTIN, r, g, 0);
#endif
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\nDJ Doorbell — link test, OUTSIDE (sender)");
  radioInit();
  addPeer(kBroadcast);
  esp_now_register_send_cb(onSent);
  esp_now_register_recv_cb(onRecv);
  led(16, 0);
}

void loop() {
  static uint32_t next = 0, lastLed = 0;
  static uint8_t turn = 0;
  const uint32_t now = millis();

  if (needPeer) {
    addPeer(insideMac);
    haveInside = true;
    needPeer = false;
    Serial.printf("inside unit found: %02X:%02X:%02X:%02X:%02X:%02X\n", insideMac[0],
                  insideMac[1], insideMac[2], insideMac[3], insideMac[4], insideMac[5]);
  }

  if (int32_t(now - next) >= 0) {
    next = now + kPeriodMs;
    // Alternate RAW / UNI; until the inside unit has answered, RAW only.
    const Kind k = (turn++ & 1) && haveInside ? UNI : RAW;
    Packet p = {};
    p.magic = kMagic;
    p.kind = k;
    p.seq = seq[k]++;
    portENTER_CRITICAL(&mux);
    if (k == UNI) uniSent++;
    p.uniSent = uniSent;  p.uniOk = uniOk;  p.uniFail = uniFail;  p.echoRx = echoRx;
    p.rttAvgUs = echoRx ? uint32_t(rttSumUs / echoRx) : 0;
    p.rttMaxUs = rttMaxUs;
    portEXIT_CRITICAL(&mux);
    p.sentUs = micros();
    esp_now_send(k == UNI ? insideMac : kBroadcast, reinterpret_cast<uint8_t*>(&p), sizeof p);
  }

  if (now - lastLed > 200) {   // green while echoes keep arriving
    lastLed = now;
    const bool ok = haveInside && now - lastEchoMs < 500;
    led(ok ? 0 : 16, ok ? 16 : 0);
  }
}

// ============================================================================
#elif defined(UNIT_INSIDE)

struct KindStats {
  bool started = false;
  uint32_t last = 0, rx = 0, lost = 0;
};

static KindStats ks[2];
static uint8_t  outsideMac[6];
static volatile bool haveOutside = false, needPeer = false;
static int32_t  rssiSum = 0, rssiN = 0, rssiMin = 0;
static uint32_t lastRxMs = 0, longestSilenceMs = 0, firstRxMs = 0;
static Packet   lastSeen;     // sender's latest self-report

static void onRecv(const esp_now_recv_info_t* info, const uint8_t* data, int len) {
  if (len != sizeof(Packet)) return;
  Packet p;
  memcpy(&p, data, sizeof p);
  if (p.magic != kMagic || p.kind > UNI) return;
  const uint32_t now = millis();
  portENTER_CRITICAL(&mux);
  KindStats& s = ks[p.kind];
  if (!s.started) { s.started = true; }
  else if (p.seq > s.last) s.lost += p.seq - s.last - 1;
  s.last = p.seq;
  s.rx++;
  const int rssi = info->rx_ctrl->rssi;
  rssiSum += rssi;
  if (rssiN++ == 0 || rssi < rssiMin) rssiMin = rssi;
  if (firstRxMs == 0) firstRxMs = now;
  else if (now - lastRxMs > longestSilenceMs) longestSilenceMs = now - lastRxMs;
  lastRxMs = now;
  lastSeen = p;
  if (!haveOutside) { memcpy(outsideMac, info->src_addr, 6); needPeer = true; }
  portEXIT_CRITICAL(&mux);

  // Echo straight back (unicast) so the sender can measure the reverse link.
  if (haveOutside) {
    p.kind = ECHO;
    esp_now_send(outsideMac, reinterpret_cast<uint8_t*>(&p), sizeof p);
  }
}

static float pct(uint32_t part, uint32_t whole) { return whole ? 100.0f * part / whole : 0; }

static void report(bool final) {
  KindStats s[2];
  Packet o;
  int32_t rAvg, rMin;
  uint32_t silence, first;
  portENTER_CRITICAL(&mux);
  s[0] = ks[0];  s[1] = ks[1];  o = lastSeen;
  rAvg = rssiN ? rssiSum / rssiN : 0;  rMin = rssiMin;
  silence = longestSilenceMs;  first = firstRxMs;
  if (final && millis() - lastRxMs > silence) silence = millis() - lastRxMs;  // silent at the end
  portEXIT_CRITICAL(&mux);

  const uint32_t t = first ? (millis() - first) / 1000 : 0;
  const uint32_t uniTried = o.uniOk + o.uniFail;
  Serial.printf("%4us  RAW rx %6u lost %5u (%5.2f%%)  UNI rx %6u lost %4u (%5.2f%%)  "
                "RSSI avg %d min %d dBm  silence max %u ms  | sender: UNI ok %u fail %u  "
                "echo %u  rtt avg %.1f max %.1f ms\n",
                t, s[0].rx, s[0].lost, pct(s[0].lost, s[0].rx + s[0].lost), s[1].rx,
                s[1].lost, pct(s[1].lost, s[1].rx + s[1].lost), rAvg, rMin, silence,
                o.uniOk, o.uniFail, o.echoRx, o.rttAvgUs / 1000.0f, o.rttMaxUs / 1000.0f);
  if (!final) return;

  // Pass criteria (proposed; the spec only says "measure packet loss"):
  //  - unicast delivery >= 99.9 %   real events go unicast with MAC retries
  //  - longest silence  < 1000 ms   less than one bar (1.94 s) without contact
  //  - average RSSI     >= -80 dBm  margin for a door that is sometimes open,
  //                                 people in the hallway, a wet coat...
  const float deliv = 100.0f - pct(o.uniFail, uniTried);
  const bool p1 = uniTried > 1000 && deliv >= 99.9f;
  const bool p2 = silence < 1000;
  const bool p3 = rAvg >= -80;
  Serial.println("\n================ LINK TEST RESULT ================");
  Serial.printf("Unicast delivery  %7.3f %%   (>= 99.9)    %s\n", deliv, p1 ? "PASS" : "FAIL");
  Serial.printf("Longest silence   %7u ms  (< 1000)     %s\n", silence, p2 ? "PASS" : "FAIL");
  Serial.printf("Average RSSI      %7d dBm (>= -80)     %s\n", rAvg, p3 ? "PASS" : "FAIL");
  Serial.printf("Raw broadcast loss %6.2f %%   (info: no retries)\n",
                pct(s[0].lost, s[0].rx + s[0].lost));
  Serial.printf("Round trip        %7.1f ms avg, %.1f ms max (info)\n",
                o.rttAvgUs / 1000.0f, o.rttMaxUs / 1000.0f);
  Serial.printf("\nOVERALL: %s\n", p1 && p2 && p3 ? "PASS — ESP-NOW is good through the door"
                                                  : "FAIL — see CLAUDE.md phase 3");
  Serial.println("==================================================");
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\nDJ Doorbell — link test, INSIDE (receiver)");
  radioInit();
  esp_now_register_recv_cb(onRecv);
  Serial.println("Waiting for the outside unit...");
}

void loop() {
  static uint32_t lastReport = 0;
  static bool done = false;

  if (needPeer) {
    addPeer(outsideMac);
    haveOutside = true;
    needPeer = false;
    Serial.printf("outside unit found: %02X:%02X:%02X:%02X:%02X:%02X — 10 min test started\n",
                  outsideMac[0], outsideMac[1], outsideMac[2], outsideMac[3], outsideMac[4],
                  outsideMac[5]);
  }
  if (!firstRxMs || done) { delay(10); return; }

  const uint32_t now = millis();
  if (now - lastReport >= kReportMs) {
    lastReport = now;
    report(false);
  }
  if (now - firstRxMs >= kTestMs) {
    report(true);
    done = true;
  }
  delay(10);
}

#endif  // UNIT_INSIDE
#endif  // LINK_TEST
