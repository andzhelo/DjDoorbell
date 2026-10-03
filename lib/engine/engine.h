// Audio engine — port of Engine in tools/simulator.py.
//
// One looping bed voice plus one retriggerable one-shot voice per pad, mixed
// to mono int16. The only clock is the count of samples rendered, which the
// firmware writes straight to I2S, so time here is I2S sample time.
//
// Two roles:
//   leader   (outside unit) — press() owns the grid: the first press starts
//            the bed and anchors the grid, the bed stops after 8 s idle.
//   follower (inside unit)  — the grid belongs to the leader. followStart()
//            / followSeek() / followStop() track the leader's bed position,
//            schedule() plays a press at the musical position the leader
//            chose. No timeout of its own.
// "Musical position" = samples since the bed started (position()); it is
// the same number on both units for the same beat.
//
// No hardware calls: this module builds and runs on the host (pio test -e native).
// Not thread-safe — call everything from the same task.
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
constexpr float    kCeiling    = 1.0f;    // never exceeds full scale, so no hard clip
constexpr uint64_t kBedTimeout = 8ull * kSampleRate;   // bed stops after 8 s idle
// A press on the same pad within this window plays the pad's repeat variant
// (if it has one) — fake dynamics, since the switches are binary.
constexpr uint64_t kRepeatWindow = 2 * kGrid;   // two 16ths, 242 ms

class Engine {
 public:
  // Called when a queued press actually starts sounding. `at` is the exact
  // sample index; with quantise on it is always a 16th boundary.
  using FireFn = void (*)(void* ctx, int pad, uint64_t at);

  // What press() decided, so the leader can tell the follower.
  struct Press {
    bool     queued;     // false: bad pad or queue full, nothing will play
    bool     variant;    // the repeat variant was chosen
    int64_t  pos;        // musical position it will sound at
    uint32_t session;    // bed run it belongs to
  };

  Engine(const Sample& bed, const Sample (&pads)[kNumPads]);

  // ---- leader ----
  // Queue pad `pad` (0-8) for the next 16th boundary. Starts the bed if it
  // is idle, anchoring the grid to this instant.
  Press press(int pad);

  // ---- follower ----
  // Start the bed so that position() == pos right now. A negative pos means
  // the bed starts -pos samples from now.
  void followStart(int64_t pos);
  // Jump musical time by `delta` samples (positive = ahead). Queued presses
  // keep their musical position.
  void followSeek(int64_t delta);
  void followStop();
  // Play `pad` at musical position `pos`. If that moment has already been
  // rendered, it plays at the next 16th instead (or now, quantise off).
  // False if the bed is not running or the queue is full.
  bool schedule(int pad, bool variant, int64_t pos);

  // ---- both ----
  // Stop the bed and every voice, then play `s` alone, unquantised.
  void chime(const Sample& s);

  // Alternate sample for `pad`, used when it is pressed again within
  // kRepeatWindow of its previous press.
  void setRepeatVariant(int pad, const Sample& alt);

  // Render `frames` mono samples and advance the clock by the same amount.
  void render(int16_t* out, size_t frames);

  uint64_t nextGrid() const;
  uint64_t now() const { return now_; }
  int64_t  position() const { return static_cast<int64_t>(now_) - bedStart_; }
  bool     bedRunning() const { return bedRunning_; }
  uint32_t session() const { return session_; }

  void setQuantise(bool on) { quantise_ = on; }
  void setBedEnabled(bool on);
  void onFire(FireFn fn, void* ctx) { fireFn_ = fn; fireCtx_ = ctx; }

 private:
  struct Pending { uint64_t at; int pad; const Sample* src; };
  struct Voice   { const Sample* src; size_t pos; bool active; };

  static constexpr int kMaxPending = 32;

  bool enqueue(const Pending& p);
  void seekBed();   // bedPos_ from position()

  Sample bed_;
  Sample pads_[kNumPads];
  Sample variants_[kNumPads] = {};       // len 0 = none
  uint64_t padLast_[kNumPads] = {};      // time of previous press per pad
  bool     padPressed_[kNumPads] = {};   // padLast_ valid

  uint64_t now_       = 0;     // master sample counter — the only clock
  int64_t  bedStart_  = 0;     // grid anchor; > now_ while a follower waits to start
  uint64_t lastPress_ = 0;
  size_t   bedPos_    = 0;
  uint32_t session_   = 0;     // bumped on every bed start
  bool     bedRunning_ = false;
  bool     bedEnabled_ = true;
  bool     follower_   = false;
  bool     quantise_   = DJ_QUANTISE;

  Voice   voices_[kNumPads] = {};
  Sample  chimeSrc_ = {};
  Voice   chime_ = {};
  Pending pending_[kMaxPending];   // sorted by `at`
  int     pendingCount_ = 0;

  FireFn fireFn_  = nullptr;
  void*  fireCtx_ = nullptr;
};

}  // namespace dj
