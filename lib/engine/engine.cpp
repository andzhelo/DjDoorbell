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
  // Floored modulo: a follower waiting to start has a negative position.
  const int64_t g = kGrid;
  const int64_t phase = ((position() % g) + g) % g;
  return now_ + static_cast<uint64_t>(g - phase);
}

bool Engine::enqueue(const Pending& p) {
  if (pendingCount_ == kMaxPending) return false;   // 32 presses inside one 16th: drop
  int i = pendingCount_++;
  // Insert after entries with the same target, keeping press order.
  while (i > 0 && pending_[i - 1].at > p.at) {
    pending_[i] = pending_[i - 1];
    --i;
  }
  pending_[i] = p;
  return true;
}

Engine::Press Engine::press(int pad) {
  if (pad < 0 || pad >= kNumPads) return {false, false, 0, session_};
  if (!bedRunning_ && bedEnabled_) {
    bedRunning_ = true;
    bedPos_ = 0;
    bedStart_ = static_cast<int64_t>(now_);
    ++session_;
  }
  lastPress_ = now_;

  const bool repeat = padPressed_[pad] && now_ - padLast_[pad] < kRepeatWindow;
  padPressed_[pad] = true;
  padLast_[pad] = now_;
  const bool variant = repeat && variants_[pad].len;
  const Sample* src = variant ? &variants_[pad] : &pads_[pad];

  const uint64_t at = nextGrid();
  const bool ok = enqueue({at, pad, src});
  return {ok, variant, static_cast<int64_t>(at) - bedStart_, session_};
}

void Engine::seekBed() {
  const int64_t pos = position();
  bedPos_ = (pos <= 0 || bed_.len == 0) ? 0 : static_cast<size_t>(pos % bed_.len);
}

void Engine::followStart(int64_t pos) {
  follower_ = true;
  bedRunning_ = bedEnabled_;
  bedStart_ = static_cast<int64_t>(now_) - pos;
  seekBed();
}

void Engine::followSeek(int64_t delta) {
  bedStart_ -= delta;
  seekBed();
  for (int i = 0; i < pendingCount_; ++i) {
    const int64_t at = static_cast<int64_t>(pending_[i].at) - delta;
    pending_[i].at = at < static_cast<int64_t>(now_) ? now_ : static_cast<uint64_t>(at);
  }
}

void Engine::followStop() { bedRunning_ = false; }

bool Engine::schedule(int pad, bool variant, int64_t pos) {
  if (pad < 0 || pad >= kNumPads || !bedRunning_) return false;
  const Sample* src = (variant && variants_[pad].len) ? &variants_[pad] : &pads_[pad];
  const int64_t at = bedStart_ + pos;
  const uint64_t when = at >= static_cast<int64_t>(now_) ? static_cast<uint64_t>(at) : nextGrid();
  return enqueue({when, pad, src});
}

void Engine::chime(const Sample& s) {
  bedRunning_ = false;
  pendingCount_ = 0;
  for (Voice& v : voices_) v.active = false;
  chimeSrc_ = s;
  chime_ = {&chimeSrc_, 0, s.len > 0};
}

void Engine::setRepeatVariant(int pad, const Sample& alt) {
  if (pad >= 0 && pad < kNumPads) variants_[pad] = alt;
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
      const Sample* src = pending_[fired].src;
      voices_[pad] = {src, 0, src->len > 0};
      if (fireFn_) fireFn_(fireCtx_, pad, t);
      ++fired;
    }
    if (fired) {
      for (int i = fired; i < pendingCount_; ++i) pending_[i - fired] = pending_[i];
      pendingCount_ -= fired;
    }

    float acc = 0.0f;
    if (bedRunning_ && bed_.len && static_cast<int64_t>(t) >= bedStart_) {
      acc += bed_.data[bedPos_] * kBedGain;
      if (++bedPos_ == bed_.len) bedPos_ = 0;
    }
    for (int i = 0; i < kNumPads; ++i) {
      Voice& v = voices_[i];
      if (!v.active) continue;
      acc += v.src->data[v.pos] * kShotGain;
      if (++v.pos == v.src->len) v.active = false;
    }
    if (chime_.active) {
      acc += chime_.src->data[chime_.pos] * kShotGain;
      if (++chime_.pos == chime_.src->len) chime_.active = false;
    }

    // Headroom + soft limit. |tanh| < 1 and kCeiling <= 1, so this never clips.
    const float y = std::tanh(acc * (kDrive / 32768.0f)) * kCeiling;
    out[n] = static_cast<int16_t>(std::lrintf(y * 32767.0f));
  }

  // Checked once per block against the block's start time, as in simulator.py.
  // A follower's bed is stopped by its leader, never by its own timeout.
  if (bedRunning_ && !follower_ && now_ - lastPress_ > kBedTimeout) bedRunning_ = false;

  now_ += frames;
}

}  // namespace dj
