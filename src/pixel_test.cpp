// Pixel chain test (pio run -e outside_pixeltest). Walks one lit pixel through
// 1-9 in red, then green, then blue, and repeats. Run on the bench-soldered
// chain before seating any pixel in a button: a dead or dim pixel, or a chain
// that stops partway, points at the joint just before it.
#ifdef PIXEL_TEST

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

constexpr uint8_t  kPinPixels = 13;
constexpr uint16_t kNumPixels = 9;
constexpr uint8_t  kLevel     = 51;    // 20% of 255
constexpr uint32_t kStepMs    = 300;

static Adafruit_NeoPixel strip(kNumPixels, kPinPixels, NEO_GRB + NEO_KHZ800);

struct Colour { const char* name; uint8_t r, g, b; };
static const Colour kColours[] = {
  {"red",   kLevel, 0, 0},
  {"green", 0, kLevel, 0},
  {"blue",  0, 0, kLevel},
};

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.printf("\nDJ Doorbell — pixel test: %u x WS2812 on GPIO%u, 20%%\n",
                kNumPixels, kPinPixels);
  // Hold the I2S pins low so a connected MAX98357A sees no clock and shuts
  // down; left floating they pick up the pixel data and the speaker roars.
  for (uint8_t pin : {10, 11, 12}) { pinMode(pin, OUTPUT); digitalWrite(pin, LOW); }
  strip.begin();
  strip.clear();
  strip.show();
}

void loop() {
  for (const Colour& c : kColours) {
    for (uint16_t i = 0; i < kNumPixels; ++i) {
      strip.clear();
      strip.setPixelColor(i, c.r, c.g, c.b);
      Serial.printf("%-5s pixel %u\n", c.name, i + 1);
      // Resend the frame continuously for the whole step so the data pin
      // carries a steady stream: a meter on GPIO13 then reads a few hundred
      // mV average, which is how to prove the board is driving the chain.
      const uint32_t until = millis() + kStepMs;
      while ((int32_t)(millis() - until) < 0) strip.show();
    }
  }
}

#endif  // PIXEL_TEST
