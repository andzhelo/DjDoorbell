// Doorbell firmware, both units.
//   outside: nine pads, pixel chain, quantised playback; leads the grid and
//            sends every press to the inside unit. Long-press pad 5 = chime.
//   inside:  plays the same presses on the same grid, 30 ms behind, locked
//            to the outside unit's bed. Volume pot on GPIO34. BOOT = chime.
#if !defined(PIXEL_TEST) && !defined(LINK_TEST)   // test builds provide their own setup()/loop()

#include <Arduino.h>
#include <ESP_I2S.h>
#include <esp_timer.h>

#include "debounce.h"
#include "engine.h"
#include "follower.h"
#include "hold.h"
#include "protocol.h"
#include "radio.h"
#include "samples.h"

#if defined(UNIT_OUTSIDE)
#include <Adafruit_NeoPixel.h>

constexpr int8_t kPinBclk = 12;
constexpr int8_t kPinWs   = 11;
constexpr int8_t kPinDout = 10;
constexpr const char* kUnitName = "outside";
constexpr const char* kHostname = "djdoorbell-outside";

// Pads 1-9 in reading order.
constexpr uint8_t  kPadPins[dj::kNumPads] = {4, 5, 6, 7, 15, 16, 17, 18, 8};
// Chain position of each pad's LED, as built: rows 1 and 3 are swapped end
// for end (pad 1 lit LED 9), the middle row is in reading order.
constexpr uint8_t  kPixelOf[dj::kNumPads] = {8, 7, 6, 3, 4, 5, 2, 1, 0};
constexpr uint32_t kDebounceMs = 5;
constexpr uint32_t kHoldMs     = 1000;   // centre pad held this long = plain chime
constexpr uint8_t  kPinPixels  = 13;
// Bench aids: the DevKit's BOOT button doubles as pad 5 (short = stab, hold =
// chime), and typing 1-9 on the serial monitor presses that pad, c = chime.
constexpr uint8_t  kPinBoot    = 0;

// Row colours as sRGB hex, from tools/simulator.py.
constexpr uint32_t kRowHex[3] = {
  0xE09030,   // row 1 amber
  0x37AFC4,   // row 2 cyan
  0xC43F81,   // row 3 magenta
};
constexpr uint8_t kBrightnessPct = 40;   // cap: power budget and PTC hold current
// kRowHex converted for the LEDs in setupInputs().
static uint32_t rowColour[3];
constexpr uint32_t kFlashMs = 110;     // as in the simulator
// Audio is rendered ahead of the speaker by the I2S DMA queue (6 x 240
// frames) plus about one block; delay the flash by the same so light and
// sound land together.
constexpr uint32_t kOutputLatencyMs = (6 * 240 + 256 / 2) * 1000 / 22050;

// Bed state to the inside unit: often while playing (each one is a lock
// measurement), rarely when idle (keeps the link alive and tells it to stop).
constexpr uint32_t kSyncPlayingMs = 100;
constexpr uint32_t kSyncIdleMs    = 1000;

static Adafruit_NeoPixel pixels(dj::kNumPads, kPinPixels, NEO_GRB + NEO_KHZ800);

#elif defined(UNIT_INSIDE)
constexpr int8_t kPinBclk = 26;
constexpr int8_t kPinWs   = 25;
constexpr int8_t kPinDout = 22;
constexpr const char* kUnitName = "inside";
constexpr const char* kHostname = "djdoorbell-inside";

constexpr uint8_t  kPinBoot    = 0;    // BOOT button, active low
constexpr uint32_t kDebounceMs = 30;
constexpr uint8_t  kPinVolume  = 34;   // pot wiper, ADC1 (ADC2 is unusable with WiFi on)
// Volume knob on/off. Off: fixed volume, GPIO34 ignored. Off until the pot
// is wired and reads correctly; build with -DDJ_VOLUME_POT=1 to turn it on.
#ifndef DJ_VOLUME_POT
#define DJ_VOLUME_POT 0
#endif
constexpr int kFixedVolumePct = 100;
// No packet from the outside unit for this long: stop the bed.
constexpr uint64_t kLostSamples = 3 * dj::kSampleRate;
#else
#error "Build with -DUNIT_OUTSIDE or -DUNIT_INSIDE"
#endif

constexpr size_t kBlock = 256;         // frames per render, 11.6 ms

static I2SClass i2s;
static dj::Engine engine(samples::kBed, samples::kPads);

// loop() / WiFi task -> audio task. The engine is not thread-safe, so
// everything that touches it is a command executed on the audio task.
enum CmdKind : uint8_t { kCmdPress, kCmdChime, kCmdPacket };
struct Cmd {
  CmdKind    kind;
  int        pad;
  dj::Packet pkt;    // kCmdPacket
  int64_t    rxUs;   // kCmdPacket: esp_timer time it arrived
};
static QueueHandle_t cmdQueue;

struct FireEvent { int pad; uint64_t at; };
static QueueHandle_t fireQueue;    // audio task -> loop(): pixels, log

// 0..32768, written by loop(), ramped by the audio task.
static volatile int32_t volumeQ15 = 32768;

static void sendCmd(const Cmd& c) { xQueueSend(cmdQueue, &c, 0); }

// Samples rendered vs esp_timer (40 MHz crystal), for the measured I2S rate
// in the status line. The two units' rates differ; the follower absorbs it.
static volatile uint64_t rateSamples = 0;
static volatile int64_t  rateUs = 0;

// Averaged since the first call: one 256-sample block is 1160 ppm of a 10 s
// window, so short windows are mostly block-timing noise.
static void printRate() {
  static uint64_t s0 = 0;
  static int64_t u0 = 0;
  const uint64_t s1 = rateSamples;
  const int64_t u1 = rateUs;
  if (!u0) { s0 = s1; u0 = u1; return; }
  if (u1 > u0) {
    const double hz = double(s1 - s0) * 1e6 / double(u1 - u0);
    Serial.printf("I2S %.2f Hz (%+.0f ppm) | ", hz, (hz / dj::kSampleRate - 1) * 1e6);
  }
}

// ---- outside: the leader ---------------------------------------------------
#if defined(UNIT_OUTSIDE)

// What the audio task tells loop() to send. pos/us pin the leader's musical
// position to a moment, so loop() can extrapolate to when it actually sends.
struct NetEvent {
  dj::PacketType type;
  int      pad;
  bool     variant;
  int64_t  at;
  uint32_t session;
  int64_t  pos;
  int64_t  us;
};
static QueueHandle_t netQueue;

// Leader's bed state, refreshed by the audio task every block.
struct Clock { int64_t pos; int64_t us; uint32_t session; bool running; };
static Clock clockNow;
static portMUX_TYPE clockMux = portMUX_INITIALIZER_UNLOCKED;

static void handleCmd(const Cmd& c, int64_t t) {
  if (c.kind == kCmdPress) {
    const dj::Engine::Press r = engine.press(c.pad);
    if (!r.queued) return;
    const NetEvent ev{dj::kPress, c.pad, r.variant, r.pos, r.session, engine.position(), t};
    xQueueSend(netQueue, &ev, 0);
  } else if (c.kind == kCmdChime) {
    engine.chime(samples::kChime);
    const NetEvent ev{dj::kChime, 0, false, 0, engine.session(), 0, t};
    xQueueSend(netQueue, &ev, 0);
  }
}

static void afterCmds(int64_t t) {
  portENTER_CRITICAL(&clockMux);
  clockNow = {engine.position(), t, engine.session(), engine.bedRunning()};
  portEXIT_CRITICAL(&clockMux);
}

static int blockAdjust() { return 0; }

// ---- inside: the follower --------------------------------------------------
#else

static dj::Follower follower;
static bool     following = false;
static uint32_t session = 0;
static uint32_t chimedSession = UINT32_MAX;   // ignore stale packets of a chimed run
static uint64_t lastRx = 0;                   // engine time of the last packet
static uint16_t lastSeq = 0;
static uint32_t lastSeqSession = UINT32_MAX;
static volatile uint32_t statPackets = 0, statPresses = 0, statLate = 0;
static volatile float statError = 0;

static void handlePacket(const dj::Packet& p, int64_t rxUs, int64_t t) {
  statPackets = statPackets + 1;
  lastRx = engine.now();

  // A resend of a press or chime the MAC layer already delivered.
  const bool once = p.type == dj::kPress || p.type == dj::kChime;
  if (once && p.seq == lastSeq && p.session == lastSeqSession) return;
  if (once) { lastSeq = p.seq; lastSeqSession = p.session; }

  if (p.type == dj::kChime) {
    engine.chime(samples::kChime);
    following = false;
    chimedSession = p.session;
    return;
  }
  if (!(p.flags & dj::kFlagRunning) || p.session == chimedSession) {
    if (following) engine.followStop();
    following = false;
    return;
  }

  const int64_t target = dj::followTarget(p.pos, rxUs, t);
  if (!following || p.session != session) {
    engine.followStart(target);
    follower.reset();
    session = p.session;
    following = true;
  } else {
    const int64_t seek = follower.observe(target - engine.position());
    if (seek) {
      engine.followSeek(seek);
      Serial.printf("sync: jumped %+lld samples\n", (long long)seek);
    }
  }
  statError = follower.error();

  if (p.type == dj::kPress) {
    statPresses = statPresses + 1;
    if (p.at < engine.position()) statLate = statLate + 1;   // already rendered: snaps to the next 16th
    engine.schedule(p.pad, p.flags & dj::kFlagVariant, p.at);
  }
}

static void handleCmd(const Cmd& c, int64_t t) {
  if (c.kind == kCmdPacket) {
    handlePacket(c.pkt, c.rxUs, t);
  } else if (c.kind == kCmdChime) {   // BOOT button: local test
    engine.chime(samples::kChime);
    following = false;
  }
}

static void afterCmds(int64_t) {
  if (following && engine.now() - lastRx > kLostSamples) {
    engine.followStop();
    following = false;
  }
}

static int blockAdjust() { return following ? follower.adjust() : 0; }

#endif

// Owns the engine. i2s.write() blocks until DMA has room, so this loop is
// paced by the I2S clock and engine.now() counts I2S samples exactly.
static void audioTask(void*) {
  static int16_t mono[kBlock + 1];
  static int16_t stereo[kBlock * 2];
  int32_t gain = 0;
  for (;;) {
    // Right after write() returns, the render head is a fixed distance
    // ahead of the speaker, so this timestamp and engine.position() belong
    // together — on both units alike.
    const int64_t t = esp_timer_get_time();
    rateSamples = rateSamples + kBlock;   // played, not rendered: the follower's slews excluded
    rateUs = t;
    Cmd c;
    while (xQueueReceive(cmdQueue, &c, 0) == pdTRUE) handleCmd(c, t);
    afterCmds(t);

    // The follower slews by rendering one sample more (dropped) or one
    // fewer (the last one repeated) than it plays.
    const int adj = blockAdjust();
    engine.render(mono, kBlock + adj);
    if (adj < 0) mono[kBlock - 1] = mono[kBlock - 2];

    const int32_t target = radio::otaActive() ? 0 : volumeQ15;
    for (size_t i = 0; i < kBlock; ++i) {
      gain += (target - gain) / 64;   // ~3 ms ramp, no zipper noise
      const int16_t s = static_cast<int16_t>((int32_t(mono[i]) * gain) >> 15);
      stereo[2 * i] = stereo[2 * i + 1] = s;
    }
    i2s.write(reinterpret_cast<const uint8_t*>(stereo), sizeof stereo);
  }
}

// Runs on the audio task: never block or print here, just hand off to loop().
// A full queue drops the event, not audio.
static void onFire(void*, int pad, uint64_t at) {
  const FireEvent ev{pad, at};
  xQueueSend(fireQueue, &ev, 0);
}

// ---- inputs ----------------------------------------------------------------
// millis() below is for debounce, LEDs and packet pacing only — never the
// musical clock.

#if defined(UNIT_OUTSIDE)
static dj::Debouncer debouncers[dj::kNumPads] = {
  dj::Debouncer(kDebounceMs), dj::Debouncer(kDebounceMs), dj::Debouncer(kDebounceMs),
  dj::Debouncer(kDebounceMs), dj::Debouncer(kDebounceMs), dj::Debouncer(kDebounceMs),
  dj::Debouncer(kDebounceMs), dj::Debouncer(kDebounceMs), dj::Debouncer(kDebounceMs),
};
static dj::Hold centreHold(kHoldMs);

// Hex colours are gamma-encoded for screens; WS2812 PWM is linear. Sending
// them raw over-drives the weaker channels and washes amber out to white.
// Decode to linear first, then apply the brightness cap.
static uint32_t ledColour(uint32_t hex) {
  auto ch = [](uint32_t hex, int shift) -> uint8_t {
    return Adafruit_NeoPixel::gamma8((hex >> shift) & 0xFF) * kBrightnessPct / 100;
  };
  return Adafruit_NeoPixel::Color(ch(hex, 16), ch(hex, 8), ch(hex, 0));
}

static void setupInputs() {
  for (uint8_t pin : kPadPins) pinMode(pin, INPUT_PULLUP);
  pinMode(kPinBoot, INPUT_PULLUP);
  for (int r = 0; r < 3; ++r) rowColour[r] = ledColour(kRowHex[r]);
  pixels.begin();
  pixels.clear();
  pixels.show();
}

// Per-pad flash window, in millis(). Zero = idle.
static uint32_t flashOn[dj::kNumPads];
static uint32_t flashOff[dj::kNumPads];
static uint32_t chimeOn = 0, chimeOff = 0;   // all nine lit while the chime plays

static void chime(uint32_t now) {
  sendCmd({kCmdChime, 0, {}, 0});
  chimeOn = now + kOutputLatencyMs;
  chimeOff = chimeOn + samples::kChime.len * 1000 / dj::kSampleRate;
}

static void pollInputs(uint32_t now) {
  for (int i = 0; i < dj::kNumPads; ++i) {
    bool raw = digitalRead(kPadPins[i]) == LOW;
    if (i == samples::kCentre) raw |= digitalRead(kPinBoot) == LOW;
    if (debouncers[i].update(raw, now)) {
      Serial.printf("pad %d\n", i + 1);
      sendCmd({kCmdPress, i, {}, 0});
    }
  }
  if (centreHold.update(debouncers[samples::kCentre].down(), now)) {
    Serial.println("long press: chime");
    chime(now);
  }
  while (Serial.available()) {
    const int ch = Serial.read();
    if (ch >= '1' && ch <= '9') {
      Serial.printf("pad %d (serial)\n", ch - '0');
      sendCmd({kCmdPress, ch - '1', {}, 0});
    } else if (ch == 'c') {
      Serial.println("chime (serial)");
      chime(now);
    }
  }
}

static void onFired(const FireEvent& ev, uint32_t now) {
  flashOn[ev.pad] = now + kOutputLatencyMs;
  flashOff[ev.pad] = flashOn[ev.pad] + kFlashMs;
}

static bool inWindow(uint32_t now, uint32_t on, uint32_t off) {
  return off && int32_t(now - on) >= 0 && int32_t(now - off) < 0;
}

static void updatePixels(uint32_t now) {
  const bool chiming = inWindow(now, chimeOn, chimeOff);
  bool changed = false;
  for (int i = 0; i < dj::kNumPads; ++i) {
    const bool lit = chiming || inWindow(now, flashOn[i], flashOff[i]);
    if (!lit && flashOff[i] && int32_t(now - flashOff[i]) >= 0) flashOff[i] = 0;
    const uint32_t want = lit ? rowColour[i / 3] : 0;
    if (pixels.getPixelColor(kPixelOf[i]) != want) {
      pixels.setPixelColor(kPixelOf[i], want);
      changed = true;
    }
  }
  if (changed) pixels.show();
}

static int64_t extrapolate(int64_t pos, int64_t us, int64_t nowUs) {
  return pos + (nowUs - us) * dj::kSampleRate / 1000000;
}

// Turn the audio task's events into packets, and send the bed state.
static void updateLink(uint32_t now) {
  static uint16_t seq = 0;
  static uint32_t lastSync = 0;

  NetEvent ev;
  while (xQueueReceive(netQueue, &ev, 0) == pdTRUE) {
    dj::Packet p = {};
    p.magic = dj::kLinkMagic;
    p.type = ev.type;
    p.pad = ev.pad;
    p.seq = ++seq;
    p.session = ev.session;
    if (ev.type == dj::kPress) {
      p.flags = dj::kFlagRunning | (ev.variant ? dj::kFlagVariant : 0);
      p.at = ev.at;
      p.pos = extrapolate(ev.pos, ev.us, esp_timer_get_time());
    }
    radio::send(p, true);
  }

  Clock c;
  portENTER_CRITICAL(&clockMux);
  c = clockNow;
  portEXIT_CRITICAL(&clockMux);
  if (now - lastSync >= (c.running ? kSyncPlayingMs : kSyncIdleMs)) {
    lastSync = now;
    dj::Packet p = {};
    p.magic = dj::kLinkMagic;
    p.type = dj::kSync;
    p.flags = c.running ? dj::kFlagRunning : 0;
    p.session = c.session;
    p.pos = extrapolate(c.pos, c.us, esp_timer_get_time());
    radio::send(p, false);
  }
}

static void printStatus() {
  printRate();
  radio::printStatus();
}

static void onPacket(const dj::Packet&, int64_t) {}   // the outside unit only sends

#else  // UNIT_INSIDE
static dj::Debouncer boot(kDebounceMs);

static void setupInputs() {
  pinMode(kPinBoot, INPUT_PULLUP);
  analogReadResolution(12);   // default attenuation (11 dB) covers the full 0-3.3 V pot range
}

// Pot to gain. Squared: a linear pot then sounds roughly even across its
// travel. The bottom few percent is silence.
static bool potIgnored = false;   // bench: 'f' on serial = full volume, knob ignored

static void readVolume(uint32_t now) {
  if (!DJ_VOLUME_POT) {
    volumeQ15 = potIgnored ? 32768 : kFixedVolumePct * 32768 / 100;
    return;
  }
  static uint32_t last = 0;
  static float smooth = -1;
  static int printed = -100;
  if (now - last < 20) return;
  last = now;
  const float x = analogRead(kPinVolume) / 4095.0f;
  smooth = smooth < 0 ? x : smooth + (x - smooth) * 0.2f;
  const int knob = static_cast<int>(smooth * 100 + 0.5f);
  if (abs(knob - printed) >= 5) {
    printed = knob;
    Serial.printf("knob %d%%\n", knob);
  }
  const float g = potIgnored ? 1.0f : smooth < 0.03f ? 0 : smooth * smooth;
  volumeQ15 = static_cast<int32_t>(g * 32768);
}

static void pollInputs(uint32_t now) {
  if (boot.update(digitalRead(kPinBoot) == LOW, now)) {
    Serial.println("BOOT: chime");
    sendCmd({kCmdChime, 0, {}, 0});
  }
  while (Serial.available()) {
    const int ch = Serial.read();
    if (ch == 'f') {
      potIgnored = !potIgnored;
      Serial.printf("volume: %s\n", potIgnored ? "FULL, knob ignored" : "knob");
    } else if (ch == 'c') {
      Serial.println("chime (serial)");
      sendCmd({kCmdChime, 0, {}, 0});
    }
  }
  readVolume(now);
}

static void onFired(const FireEvent&, uint32_t) {}
static void updatePixels(uint32_t) {}
static void updateLink(uint32_t) {}

static void printStatus() {
  printRate();
  Serial.printf("following %s, packets %u, presses %u (late %u), sync error %+.1f samples, "
                "slews %u, jumps %u, volume %d%% | ",
                following ? "yes" : "no", (unsigned)statPackets, (unsigned)statPresses,
                (unsigned)statLate, statError, (unsigned)follower.slews(),
                (unsigned)follower.seeks(), (int)(volumeQ15 * 100 / 32768));
  radio::printStatus();
}

// WiFi task: hand the packet to the audio task, which owns the engine.
static void onPacket(const dj::Packet& p, int64_t rxUs) {
  Cmd c{kCmdPacket, 0, p, rxUs};
  xQueueSend(cmdQueue, &c, 0);
}
#endif

// ---- main ------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(200);

  Serial.printf("\nDJ Doorbell — %s unit\n", kUnitName);
  Serial.printf("Chip:     %s rev %u, %u cores, %u MHz\n", ESP.getChipModel(),
                ESP.getChipRevision(), ESP.getChipCores(), ESP.getCpuFreqMHz());
  Serial.printf("Flash:    %u MB\n", ESP.getFlashChipSize() / (1024 * 1024));
  Serial.printf("PSRAM:    %u KB\n", ESP.getPsramSize() / 1024);
  Serial.printf("Core:     Arduino-ESP32 %s, IDF %s\n", ESP_ARDUINO_VERSION_STR,
                esp_get_idf_version());
  Serial.printf("Quantise: %s, grid %u samples\n", DJ_QUANTISE ? "on" : "off",
                (unsigned)dj::kGrid);
  samples::verify();

  i2s.setPins(kPinBclk, kPinWs, kPinDout);
  if (!i2s.begin(I2S_MODE_STD, dj::kSampleRate, I2S_DATA_BIT_WIDTH_16BIT,
                 I2S_SLOT_MODE_STEREO)) {
    Serial.println("FATAL: I2S begin failed");
    for (;;) delay(1000);
  }
  Serial.printf("I2S:      BCLK %d, WS %d, DOUT %d @ %u Hz\n", kPinBclk, kPinWs,
                kPinDout, (unsigned)dj::kSampleRate);

  setupInputs();
  engine.onFire(onFire, nullptr);
  engine.setRepeatVariant(samples::kHat, samples::kHatOpen);
  cmdQueue = xQueueCreate(32, sizeof(Cmd));
  fireQueue = xQueueCreate(32, sizeof(FireEvent));
#if defined(UNIT_OUTSIDE)
  netQueue = xQueueCreate(16, sizeof(NetEvent));
#endif
  // Core 1 alongside loop(); WiFi/ESP-NOW live on core 0.
  xTaskCreatePinnedToCore(audioTask, "audio", 4096, nullptr, 10, nullptr, 1);

  radio::begin(kHostname, onPacket);

#if defined(UNIT_OUTSIDE)
  Serial.printf("Pads:     GPIO 4 5 6 7 15 16 17 18 8, debounce %u ms, hold pad 5 %u ms = chime\n",
                (unsigned)kDebounceMs, (unsigned)kHoldMs);
  Serial.printf("Pixels:   GPIO%u, flash delayed %u ms to match audio\n",
                kPinPixels, (unsigned)kOutputLatencyMs);
  for (int r = 0; r < 3; ++r) {
    Serial.printf("Row %d:    #%06lX -> LED %3u %3u %3u\n", r + 1, (unsigned long)kRowHex[r],
                  (unsigned)(rowColour[r] >> 16 & 0xFF), (unsigned)(rowColour[r] >> 8 & 0xFF),
                  (unsigned)(rowColour[r] & 0xFF));
  }
  Serial.println("Ready — press a pad (or BOOT = pad 5, or type 1-9 / c here).");
#else
  if (DJ_VOLUME_POT) Serial.printf("Volume:   pot on GPIO%u\n", kPinVolume);
  else Serial.printf("Volume:   fixed %d%% (knob off, DJ_VOLUME_POT=0)\n", kFixedVolumePct);
  Serial.println("Ready — waiting for the outside unit. BOOT plays the chime.");
#endif
}

void loop() {
  static uint32_t lastStatus = 0;
  const uint32_t now = millis();
  pollInputs(now);
  radio::loop();
  updateLink(now);

  FireEvent ev;
  while (xQueueReceive(fireQueue, &ev, 0) == pdTRUE) {
    Serial.printf("fire pad %d @ %llu\n", ev.pad + 1, (unsigned long long)ev.at);
    onFired(ev, now);
  }
  updatePixels(now);

  if (now - lastStatus >= 10000) {
    lastStatus = now;
    printStatus();
  }
  delay(1);
}

#endif  // !PIXEL_TEST && !LINK_TEST
