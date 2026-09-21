#include "engine.h"

#include <cmath>

namespace dj {

Engine::Engine(const Sample& bed, const Sample (&pads)[kNumPads]) : bed_(bed) {
  for (int i = 0; i < kNumPads; ++i) pads_[i] = pads[i];
}

uint64_t Engine::nextGrid() const {
  if (!quantise_ || !bedRunning_) return now_;
  // Strictly the next boundary, as in simulator.py: a press exactly on a
  // boundary waits a full 16th, because that boundary is already rendered.
  const uint64_t phase = (now_ - bedStart_) % kGrid;
  return now_ + (kGrid - phase);
}

void Engine::press(int pad) {
  if (pad < 0 || pad >= kNumPads) return;
  if (!bedRunning_ && bedEnabled_) {
    bedRunning_ = true;
    bedPos_ = 0;
    bedStart_ = now_;
  }
  lastPress_ = now_;
  if (pendingCount_ == kMaxPending) return;   // 32 presses inside one 16th: drop

  const Pending p{nextGrid(), pad};
  int i = pendingCount_++;
  // Insert after entries with the same target, keeping press order.
  while (i > 0 && pending_[i - 1].at > p.at) {
    pending_[i] = pending_[i - 1];
    --i;
  }
  pending_[i] = p;
}

void Engine::setBedEnabled(bool on) {
  bedEnabled_ = on;
  if (!on) bedRunning_ = false;
}

void Engine::render(int16_t* out, size_t frames) {
  for (size_t n = 0; n < frames; ++n) {
    const uint64_t t = now_ + n;

    // Start any one-shots due at or before this sample. Retriggering a pad
    // restarts its voice.
    int fired = 0;
    while (fired < pendingCount_ && pending_[fired].at <= t) {
      const int pad = pending_[fired].pad;
      voices_[pad] = {0, pads_[pad].len > 0};
      if (fireFn_) fireFn_(fireCtx_, pad, t);
      ++fired;
    }
    if (fired) {
      for (int i = fired; i < pendingCount_; ++i) pending_[i - fired] = pending_[i];
      pendingCount_ -= fired;
    }

    float acc = 0.0f;
    if (bedRunning_ && bed_.len) {
      acc += bed_.data[bedPos_] * kBedGain;
      if (++bedPos_ == bed_.len) bedPos_ = 0;
    }
    for (int i = 0; i < kNumPads; ++i) {
      Voice& v = voices_[i];
      if (!v.active) continue;
      acc += pads_[i].data[v.pos] * kShotGain;
      if (++v.pos == pads_[i].len) v.active = false;
    }

    // Headroom + soft limit. The curve peaks at 1.15, so hard-clip the rest.
    float y = std::tanh(acc * (kDrive / 32768.0f)) * kCeiling;
    if (y > 1.0f) y = 1.0f;
    if (y < -1.0f) y = -1.0f;
    out[n] = static_cast<int16_t>(std::lrintf(y * 32767.0f));
  }

  // Checked once per block against the block's start time, as in simulator.py.
  if (bedRunning_ && now_ - lastPress_ > kBedTimeout) bedRunning_ = false;

  now_ += frames;
}

}  // namespace dj
