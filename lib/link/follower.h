// Keeps the follower's bed locked to the leader's.
//
// The two boards' I2S clocks come from different crystals and dividers, so
// they drift apart by up to a few hundred ppm (a few samples per second).
// Each packet gives one noisy measurement of the error (where the leader says
// we should be minus where we are); the noise is mostly the I2S DMA cadence,
// a few ms either way on each board. The error is low-pass filtered and
// corrected one sample at a time: the audio task renders one sample extra
// and drops it (catch up) or renders one fewer and repeats one (wait). One
// sample in 22050 is inaudible. Only a large error — a lost lock, a long
// radio gap — jumps straight there.
//
// No hardware calls: host-testable.
#pragma once

#include <cstdint>

namespace dj {

class Follower {
 public:
  static constexpr int64_t kSeek     = 22050 * 50 / 1000;   // 50 ms: jump
  static constexpr float   kDeadband = 22050 * 0.5f / 1000; // 0.5 ms: leave alone
  static constexpr float   kAlpha    = 1.0f / 16;           // filter, per packet (10/s)
  static constexpr int     kCooldown = 4;                   // blocks between slews

  void reset() { primed_ = false; filt_ = 0; cooldown_ = 0; }

  // One measurement, in samples (positive = we are behind). Returns a jump
  // to apply right now, 0 if none.
  int64_t observe(int64_t err) {
    if (err > kSeek || err < -kSeek) {
      ++seeks_;
      primed_ = true;
      filt_ = 0;
      return err;
    }
    filt_ = primed_ ? filt_ + (err - filt_) * kAlpha : static_cast<float>(err);
    primed_ = true;
    return 0;
  }

  // Once per audio block: +1 = render one extra sample and drop it, -1 =
  // render one fewer and repeat one, 0 = nothing.
  int adjust() {
    if (cooldown_ > 0) { --cooldown_; return 0; }
    int a = 0;
    if (filt_ > kDeadband) a = 1;
    else if (filt_ < -kDeadband) a = -1;
    if (a) { filt_ -= a; cooldown_ = kCooldown; ++slews_; }
    return a;
  }

  float    error() const { return filt_; }
  uint32_t seeks() const { return seeks_; }
  uint32_t slews() const { return slews_; }

 private:
  bool     primed_ = false;
  float    filt_ = 0;
  int      cooldown_ = 0;
  uint32_t seeks_ = 0, slews_ = 0;
};

}  // namespace dj
