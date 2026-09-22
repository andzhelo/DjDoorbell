// Switch debouncer. A new level is accepted only after the raw input has held
// it for `ms` without interruption, so contact bounce and short noise spikes
// on long button wires are both rejected. Costs `ms` of latency per press.
//
// No hardware calls: the caller samples the pin and supplies the time.
#pragma once

#include <cstdint>

namespace dj {

class Debouncer {
 public:
  explicit Debouncer(uint32_t ms) : ms_(ms) {}

  // Feed one raw sample (true = pressed). Returns true exactly once per
  // accepted press, on the update that accepts it.
  bool update(bool raw, uint32_t nowMs) {
    if (raw != candidate_) {
      candidate_ = raw;
      since_ = nowMs;
    }
    if (candidate_ != stable_ && nowMs - since_ >= ms_) {
      stable_ = candidate_;
      return stable_;
    }
    return false;
  }

  bool down() const { return stable_; }

 private:
  uint32_t ms_;
  uint32_t since_ = 0;
  bool candidate_ = false;
  bool stable_ = false;
};

}  // namespace dj
