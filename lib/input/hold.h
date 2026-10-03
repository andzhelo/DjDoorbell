// Long-press detector. Feed it a debounced level; it fires once when the
// button has been held for `ms`, and re-arms on release.
//
// No hardware calls: the caller supplies the level and the time.
#pragma once

#include <cstdint>

namespace dj {

class Hold {
 public:
  explicit Hold(uint32_t ms) : ms_(ms) {}

  bool update(bool down, uint32_t nowMs) {
    if (!down) { held_ = fired_ = false; return false; }
    if (!held_) { held_ = true; since_ = nowMs; }
    if (fired_ || nowMs - since_ < ms_) return false;
    fired_ = true;
    return true;
  }

 private:
  uint32_t ms_;
  uint32_t since_ = 0;
  bool held_ = false, fired_ = false;
};

}  // namespace dj
