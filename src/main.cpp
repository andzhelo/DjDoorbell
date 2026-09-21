// Bench firmware, phases 1-2: I2S out, sample-accurate clock, one button.
// BOOT (GPIO0) plays 04_stab_am, quantised against the loop bed.
#include <Arduino.h>
#include <ESP_I2S.h>

#include "engine.h"
#include "samples.h"

#if defined(UNIT_OUTSIDE)
constexpr int8_t kPinBclk = 12;
constexpr int8_t kPinWs   = 11;
constexpr int8_t kPinDout = 10;
constexpr const char* kUnitName = "outside";
#elif defined(UNIT_INSIDE)
constexpr int8_t kPinBclk = 26;
constexpr int8_t kPinWs   = 25;
constexpr int8_t kPinDout = 22;
constexpr const char* kUnitName = "inside";
#else
#error "Build with -DUNIT_OUTSIDE or -DUNIT_INSIDE"
#endif

constexpr uint8_t  kPinBoot   = 0;     // BOOT button, active low
constexpr size_t   kBlock     = 256;   // frames per render, 11.6 ms
constexpr uint32_t kDebounceMs = 30;

static I2SClass i2s;
static dj::Engine engine(samples::kBed, samples::kPads);
static QueueHandle_t pressQueue;

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

static void onFire(void*, int pad, uint64_t at) {
  // Runs on the audio task; keep it cheap. ets_printf avoids Serial's lock
  // (ROM printf has no %llu; the 32-bit count wraps after 54 h).
  ets_printf("fire pad %d @ %lu\n", pad + 1, (unsigned long)at);
}

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

  pinMode(kPinBoot, INPUT_PULLUP);
  engine.onFire(onFire, nullptr);
  pressQueue = xQueueCreate(16, sizeof(int));
  // Core 1 alongside loop(); WiFi/ESP-NOW will live on core 0.
  xTaskCreatePinnedToCore(audioTask, "audio", 4096, nullptr, 10, nullptr, 1);

  Serial.println("Ready — press BOOT.");
}

void loop() {
  static bool     wasDown = false;
  static uint32_t lastEdge = 0;

  const bool down = digitalRead(kPinBoot) == LOW;
  const uint32_t t = millis();   // debounce only — never the musical clock
  if (down != wasDown && t - lastEdge >= kDebounceMs) {
    wasDown = down;
    lastEdge = t;
    if (down) {
      const int pad = samples::kStabAm;
      xQueueSend(pressQueue, &pad, 0);
    }
  }
  delay(1);
}
