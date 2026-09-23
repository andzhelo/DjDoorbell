// Bench firmware.
//   outside (phase 4): nine pads, pixel chain, each press quantised over the bed.
//   inside  (phase 2): BOOT (GPIO0) plays 04_stab_am, quantised over the bed.
#ifndef PIXEL_TEST   // pixel_test.cpp provides setup()/loop() instead

#include <Arduino.h>
#include <ESP_I2S.h>

#include "debounce.h"
#include "engine.h"
#include "samples.h"

#if defined(UNIT_OUTSIDE)
#include <Adafruit_NeoPixel.h>

constexpr int8_t kPinBclk = 12;
constexpr int8_t kPinWs   = 11;
constexpr int8_t kPinDout = 10;
constexpr const char* kUnitName = "outside";

// Pads 1-9 in reading order; pixel index == pad index.
constexpr uint8_t  kPadPins[dj::kNumPads] = {4, 5, 6, 7, 15, 16, 17, 18, 8};
constexpr uint32_t kDebounceMs = 5;
constexpr uint8_t  kPinPixels  = 13;

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

static Adafruit_NeoPixel pixels(dj::kNumPads, kPinPixels, NEO_GRB + NEO_KHZ800);

#elif defined(UNIT_INSIDE)
constexpr int8_t kPinBclk = 26;
constexpr int8_t kPinWs   = 25;
constexpr int8_t kPinDout = 22;
constexpr const char* kUnitName = "inside";

constexpr uint8_t  kPinBoot    = 0;    // BOOT button, active low
constexpr uint32_t kDebounceMs = 30;
#else
#error "Build with -DUNIT_OUTSIDE or -DUNIT_INSIDE"
#endif

constexpr size_t kBlock = 256;         // frames per render, 11.6 ms

static I2SClass i2s;
static dj::Engine engine(samples::kBed, samples::kPads);
static QueueHandle_t pressQueue;   // loop() -> audio task: pad index
static QueueHandle_t fireQueue;    // audio task -> loop(): FireEvent

struct FireEvent { int pad; uint64_t at; };

// Owns the engine. i2s.write() blocks until DMA has room, so this loop is
// paced by the I2S clock and engine.now() counts I2S samples exactly.
static void audioTask(void*) {
  static int16_t mono[kBlock];
  static int16_t stereo[kBlock * 2];
  for (;;) {
    int pad;
    while (xQueueReceive(pressQueue, &pad, 0) == pdTRUE) engine.press(pad);

    engine.render(mono, kBlock);
    for (size_t i = 0; i < kBlock; ++i) stereo[2 * i] = stereo[2 * i + 1] = mono[i];
    i2s.write(reinterpret_cast<const uint8_t*>(stereo), sizeof stereo);
  }
}

// Runs on the audio task: never block or print here, just hand off to loop().
// A full queue drops the event, not audio.
static void onFire(void*, int pad, uint64_t at) {
  const FireEvent ev{pad, at};
  xQueueSend(fireQueue, &ev, 0);
}

static void press(int pad) {
  Serial.printf("pad %d\n", pad + 1);
  xQueueSend(pressQueue, &pad, 0);
}

// ---- inputs ----------------------------------------------------------------
// millis() below is for debounce and LED timing only — never the musical clock.

#if defined(UNIT_OUTSIDE)
static dj::Debouncer debouncers[dj::kNumPads] = {
  dj::Debouncer(kDebounceMs), dj::Debouncer(kDebounceMs), dj::Debouncer(kDebounceMs),
  dj::Debouncer(kDebounceMs), dj::Debouncer(kDebounceMs), dj::Debouncer(kDebounceMs),
  dj::Debouncer(kDebounceMs), dj::Debouncer(kDebounceMs), dj::Debouncer(kDebounceMs),
};

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
  for (int r = 0; r < 3; ++r) rowColour[r] = ledColour(kRowHex[r]);
  pixels.begin();
  pixels.clear();
  pixels.show();
}

static void pollInputs(uint32_t now) {
  for (int i = 0; i < dj::kNumPads; ++i) {
    if (debouncers[i].update(digitalRead(kPadPins[i]) == LOW, now)) press(i);
  }
}

// Per-pad flash window, in millis(). Zero = idle.
static uint32_t flashOn[dj::kNumPads];
static uint32_t flashOff[dj::kNumPads];

static void onFired(const FireEvent& ev, uint32_t now) {
  flashOn[ev.pad] = now + kOutputLatencyMs;
  flashOff[ev.pad] = flashOn[ev.pad] + kFlashMs;
}

static void updatePixels(uint32_t now) {
  bool changed = false;
  for (int i = 0; i < dj::kNumPads; ++i) {
    const bool lit = flashOff[i] && int32_t(now - flashOn[i]) >= 0 &&
                     int32_t(now - flashOff[i]) < 0;
    if (!lit && flashOff[i] && int32_t(now - flashOff[i]) >= 0) flashOff[i] = 0;
    const uint32_t want = lit ? rowColour[i / 3] : 0;
    if (pixels.getPixelColor(i) != want) {
      pixels.setPixelColor(i, want);
      changed = true;
    }
  }
  if (changed) pixels.show();
}

#else  // UNIT_INSIDE
static dj::Debouncer boot(kDebounceMs);

static void setupInputs() { pinMode(kPinBoot, INPUT_PULLUP); }

static void pollInputs(uint32_t now) {
  if (boot.update(digitalRead(kPinBoot) == LOW, now)) press(samples::kStabAm);
}

static void onFired(const FireEvent&, uint32_t) {}
static void updatePixels(uint32_t) {}
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
  pressQueue = xQueueCreate(16, sizeof(int));
  fireQueue = xQueueCreate(32, sizeof(FireEvent));
  // Core 1 alongside loop(); WiFi/ESP-NOW will live on core 0.
  xTaskCreatePinnedToCore(audioTask, "audio", 4096, nullptr, 10, nullptr, 1);

#if defined(UNIT_OUTSIDE)
  Serial.printf("Pads:     GPIO 4 5 6 7 15 16 17 18 8, debounce %u ms\n",
                (unsigned)kDebounceMs);
  Serial.printf("Pixels:   GPIO%u, flash delayed %u ms to match audio\n",
                kPinPixels, (unsigned)kOutputLatencyMs);
  for (int r = 0; r < 3; ++r) {
    Serial.printf("Row %d:    #%06lX -> LED %3u %3u %3u\n", r + 1, (unsigned long)kRowHex[r],
                  (unsigned)(rowColour[r] >> 16 & 0xFF), (unsigned)(rowColour[r] >> 8 & 0xFF),
                  (unsigned)(rowColour[r] & 0xFF));
  }
  Serial.println("Ready — press a pad.");
#else
  Serial.println("Ready — press BOOT.");
#endif
}

void loop() {
  const uint32_t now = millis();
  pollInputs(now);

  FireEvent ev;
  while (xQueueReceive(fireQueue, &ev, 0) == pdTRUE) {
    Serial.printf("fire pad %d @ %llu\n", ev.pad + 1, (unsigned long long)ev.at);
    onFired(ev, now);
  }
  updatePixels(now);
  delay(1);
}

#endif  // PIXEL_TEST
