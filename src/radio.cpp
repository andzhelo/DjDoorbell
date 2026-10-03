#if !defined(PIXEL_TEST) && !defined(LINK_TEST)

#include "radio.h"

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_timer.h>
#include <esp_wifi.h>

#if __has_include("secrets.h")
#include "secrets.h"
#define HAVE_WIFI 1
#else
#define HAVE_WIFI 0   // no credentials: ESP-NOW only, on LINK_CHANNEL
#endif

#ifndef LINK_CHANNEL
#define LINK_CHANNEL 1   // used only without WiFi; the router decides otherwise
#endif

namespace radio {
namespace {

constexpr uint8_t  kBroadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
constexpr uint32_t kLinkTimeoutMs = 3000;
constexpr uint32_t kWifiGiveUpMs  = 20000;
constexpr int      kResends       = 2;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
RxFn rxFn = nullptr;

uint8_t peer[6];
bool havePeer = false;                    // loop() only
volatile bool newPeer = false;            // callback -> loop()
uint8_t newPeerMac[6];
volatile uint32_t lastHeardMs = 0;        // ack (outside) or packet (inside) from the peer
volatile bool needHelloAck = false;       // inside: answer in loop()

// Unicast sends complete in order, so a ring of "was this one reliable"
// tells the send callback which packet a result belongs to.
volatile bool sentReliable[16];
volatile uint8_t sentHead = 0, sentTail = 0;
dj::Packet resendPkt;
int64_t resendUs = 0;
volatile int resendsLeft = 0;
volatile bool resendDue = false;

enum class Wifi { Off, Trying, Up };
Wifi wifi = Wifi::Off;
uint32_t wifiStartMs = 0;
bool wifiWasUp = false;
volatile bool ota = false;
uint8_t channel = LINK_CHANNEL;
uint32_t lastHelloMs = 0;

void onSent(const esp_now_send_info_t* info, esp_now_send_status_t status) {
  if (memcmp(info->des_addr, kBroadcast, 6) == 0) return;   // no ACK exists for broadcast
  portENTER_CRITICAL_ISR(&mux);
  const bool reliable = sentHead != sentTail && sentReliable[sentTail];
  if (sentHead != sentTail) sentTail = (sentTail + 1) % 16;
  if (status == ESP_NOW_SEND_SUCCESS) {
    lastHeardMs = millis();
    if (reliable) resendsLeft = 0;
  } else if (reliable && resendsLeft > 0) {
    resendDue = true;
  }
  portEXIT_CRITICAL_ISR(&mux);
}

void onRecv(const esp_now_recv_info_t* info, const uint8_t* data, int len) {
  dj::Packet p;
  if (!dj::decode(data, len, p)) return;
  const int64_t rxUs = esp_timer_get_time();
  lastHeardMs = millis();
#if defined(UNIT_OUTSIDE)
  if (p.type == dj::kHelloAck && !newPeer) {
    memcpy(newPeerMac, info->src_addr, 6);
    newPeer = true;
  }
#else
  if (p.type == dj::kHello) {
    if (!newPeer) { memcpy(newPeerMac, info->src_addr, 6); newPeer = true; }
    needHelloAck = true;
    return;
  }
  if (rxFn) rxFn(p, rxUs);
#endif
}

void addPeer(const uint8_t* mac) {
  if (esp_now_is_peer_exist(mac)) return;
  esp_now_peer_info_t p = {};
  memcpy(p.peer_addr, mac, 6);
  p.channel = 0;   // whatever channel the radio is on now: follows the router
  p.ifidx = WIFI_IF_STA;
  esp_now_add_peer(&p);
}

// Every unicast goes through here so the ring stays in step with the send
// callbacks. A send the driver refuses gets no callback: take it back out.
bool unicast(const dj::Packet& p, bool reliable) {
  portENTER_CRITICAL(&mux);
  sentReliable[sentHead] = reliable;
  sentHead = (sentHead + 1) % 16;
  portEXIT_CRITICAL(&mux);
  if (esp_now_send(peer, reinterpret_cast<const uint8_t*>(&p), sizeof p) == ESP_OK) return true;
  portENTER_CRITICAL(&mux);
  sentHead = (sentHead + 15) % 16;
  portEXIT_CRITICAL(&mux);
  return false;
}

void setChannel(uint8_t ch) {
  channel = ch;
  esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
}

void startOta() {
#if HAVE_WIFI
  ArduinoOTA.setHostname(WiFi.getHostname());
#ifdef OTA_PASSWORD
  ArduinoOTA.setPassword(OTA_PASSWORD);
#else
#warning "OTA_PASSWORD not set in secrets.h: anyone on your WiFi can reflash this unit"
#endif
  ArduinoOTA.onStart([] { ota = true; Serial.println("OTA: update started, audio muted"); });
  ArduinoOTA.onEnd([] { Serial.println("OTA: done, rebooting"); });
  ArduinoOTA.onError([](ota_error_t e) {
    ota = false;
    Serial.printf("OTA: failed (%u)\n", (unsigned)e);
  });
  ArduinoOTA.begin();
  Serial.printf("OTA:      ready, host %s.local\n", WiFi.getHostname());
#endif
}

void wifiLoop(uint32_t now) {
#if HAVE_WIFI
  const bool up = WiFi.status() == WL_CONNECTED;
  if (wifi == Wifi::Trying && up) {
    wifi = Wifi::Up;
    channel = WiFi.channel();
    Serial.printf("WiFi:     connected, %s, channel %u, RSSI %d dBm\n",
                  WiFi.localIP().toString().c_str(), channel, WiFi.RSSI());
    if (!wifiWasUp) startOta();
    wifiWasUp = true;
  } else if (wifi == Wifi::Up && !up) {
    wifi = Wifi::Trying;   // auto-reconnect is on
    wifiStartMs = now;
    Serial.println("WiFi:     lost, reconnecting");
  } else if (wifi == Wifi::Trying && !up && now - wifiStartMs > kWifiGiveUpMs) {
#if defined(UNIT_OUTSIDE)
    // Reconnect attempts scan every channel, which breaks ESP-NOW. The pad
    // matters more than OTA: give up WiFi until the next reboot.
    WiFi.setAutoReconnect(false);
    WiFi.disconnect();
    wifi = Wifi::Off;
    setChannel(LINK_CHANNEL);
    Serial.println("WiFi:     router not reachable from here; ESP-NOW only, OTA off until reboot");
#else
    wifiStartMs = now;
    Serial.println("WiFi:     still trying to connect");
#endif
  }
  if (wifi == Wifi::Up) ArduinoOTA.handle();
#else
  (void)now;
#endif
}

}  // namespace

void begin(const char* hostname, RxFn onRx) {
  rxFn = onRx;
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(hostname);
  WiFi.setSleep(false);   // modem sleep would make the inside unit miss packets
#if HAVE_WIFI
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  wifi = Wifi::Trying;
  wifiStartMs = millis();
  Serial.printf("WiFi:     connecting to \"%s\" as %s\n", WIFI_SSID, hostname);
#else
  setChannel(LINK_CHANNEL);
  Serial.printf("WiFi:     no include/secrets.h, ESP-NOW only on channel %u\n", LINK_CHANNEL);
#endif
  if (esp_now_init() != ESP_OK) {
    Serial.println("FATAL: esp_now_init failed");
    for (;;) delay(1000);
  }
  esp_now_register_send_cb(onSent);
  esp_now_register_recv_cb(onRecv);
  addPeer(kBroadcast);
  uint8_t mac[6];
  esp_wifi_get_mac(WIFI_IF_STA, mac);
  Serial.printf("ESP-NOW:  MAC %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3],
                mac[4], mac[5]);
}

void loop() {
  const uint32_t now = millis();
  wifiLoop(now);

  if (newPeer) {
    memcpy(peer, newPeerMac, 6);
    addPeer(peer);
    if (!havePeer) {
      Serial.printf("ESP-NOW:  %s unit found: %02X:%02X:%02X:%02X:%02X:%02X\n",
#if defined(UNIT_OUTSIDE)
                    "inside",
#else
                    "outside",
#endif
                    peer[0], peer[1], peer[2], peer[3], peer[4], peer[5]);
    }
    havePeer = true;
    newPeer = false;
  }

#if defined(UNIT_OUTSIDE)
  // Look for the inside unit until it answers; if it goes quiet and we are
  // not tied to the router's channel, try the next channel each time.
  if ((!havePeer || !linked()) && now - lastHelloMs > 300) {
    lastHelloMs = now;
    if (havePeer && wifi == Wifi::Off) setChannel(channel % 13 + 1);
    dj::Packet h = {};
    h.magic = dj::kLinkMagic;
    h.type = dj::kHello;
    esp_now_send(kBroadcast, reinterpret_cast<const uint8_t*>(&h), sizeof h);
  }
  if (resendDue) {
    resendDue = false;
    const int64_t t = esp_timer_get_time();
    if (resendPkt.flags & dj::kFlagRunning)   // the leader kept playing meanwhile
      resendPkt.pos += (t - resendUs) * dj::kSampleRate / 1000000;
    resendUs = t;
    resendsLeft = resendsLeft - 1;
    unicast(resendPkt, true);
  }
#else
  if (needHelloAck && havePeer) {
    needHelloAck = false;
    dj::Packet a = {};
    a.magic = dj::kLinkMagic;
    a.type = dj::kHelloAck;
    esp_now_send(peer, reinterpret_cast<const uint8_t*>(&a), sizeof a);
  }
#endif
}

bool send(const dj::Packet& p, bool reliable) {
  if (!havePeer) return false;
  portENTER_CRITICAL(&mux);
  if (reliable) {
    resendPkt = p;
    resendUs = esp_timer_get_time();
    resendsLeft = kResends;
    resendDue = false;
  }
  portEXIT_CRITICAL(&mux);
  return unicast(p, reliable);
}

bool linked() { return lastHeardMs && millis() - lastHeardMs < kLinkTimeoutMs; }

bool otaActive() { return ota; }

void printStatus() {
  Serial.printf("link %s, channel %u, wifi %s\n", linked() ? "UP" : "down",
                wifi == Wifi::Up ? WiFi.channel() : channel,
                wifi == Wifi::Up ? "up" : wifi == Wifi::Trying ? "connecting" : "off");
}

}  // namespace radio

#endif  // !PIXEL_TEST && !LINK_TEST
