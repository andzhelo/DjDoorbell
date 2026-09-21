// Audio engine — port of Engine in tools/simulator.py.
//
// One looping bed voice plus one retriggerable one-shot voice per pad, mixed
// to mono int16. The only clock is the count of samples rendered, which the
// firmware writes straight to I2S, so time here is I2S sample time.
//
// No hardware calls: this module builds and runs on the host (pio test -e native).
// Not thread-safe — call press() and render() from the same task.
#pragma once

#include <cstddef>
#include <cstdint>

#ifndef DJ_QUANTISE
#define DJ_QUANTISE 1   // compile-time A/B toggle: 0 = play presses immediately
#endif

namespace dj {

struct Sample {
  const int16_t* data;
  size_t len;           // in samples, not bytes
};

constexpr uint32_t kSampleRate = 22050;
constexpr int      kNumPads    = 9;
// int(S16 * SR) in simulator.py: 60 / 124 / 4 * 22050 = 2667.3 -> 2667 (121 ms)
constexpr uint32_t kGrid = static_cast<uint32_t>(60.0 / 124.0 / 4.0 * kSampleRate);
static_assert(kGrid == 2667, "16th grid at 124 BPM / 22050 Hz must be 2667 samples");

constexpr float    kBedGain    = 0.80f;
constexpr float    kShotGain   = 0.85f;
constexpr float    kDrive      = 0.62f;   // tanh(x * kDrive) * kCeiling
constexpr float    kCeiling    = 1.15f;
constexpr uint64_t kBedTimeout = 8ull * kSampleRate;   // bed stops after 8 s idle

class Engine {
 public:
  // Called when a queued press actually starts sounding. `at` is the exact
  // sample index; with quantise on it is always a 16th boundary.
  using FireFn = void (*)(void* ctx, int pad, uint64_t at);

  Engine(const Sample& bed, const Sample (&pads)[kNumPads]);

  // Queue pad `pad` (0-8) for the next 16th boundary. Starts the bed if it
  // is idle, anchoring the grid to this instant.
  void press(int pad);

  // Render `frames` mono samples and advance the clock by the same amount.
  void render(int16_t* out, size_t frames);

  uint64_t nextGrid() const;
  uint64_t now() const { return now_; }
  bool bedRunning() const { return bedRunning_; }

  void setQuantise(bool on) { quantise_ = on; }
  void setBedEnabled(bool on);
  void onFire(FireFn fn, void* ctx) { fireFn_ = fn; fireCtx_ = ctx; }

 private:
  struct Pending { uint64_t at; int pad; };
  struct Voice   { size_t pos; bool active; };

  static constexpr int kMaxPending = 32;

  Sample bed_;
  Sample pads_[kNumPads];

  uint64_t now_       = 0;     // master sample counter — the only clock
  uint64_t bedStart_  = 0;     // grid anchor
  uint64_t lastPress_ = 0;
  size_t   bedPos_    = 0;
  bool     bedRunning_ = false;
  bool     bedEnabled_ = true;
  bool     quantise_   = DJ_QUANTISE;

  Voice   voices_[kNumPads] = {};
  Pending pending_[kMaxPending];   // sorted by `at`
  int     pendingCount_ = 0;

  FireFn fireFn_  = nullptr;
  void*  fireCtx_ = nullptr;
};

}  // namespace dj
