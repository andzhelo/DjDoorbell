// Host tests for the two-board sync: engine follower mode, the lock loop,
// the wire format and the long-press detector.  pio test -e native
#include <unity.h>

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <vector>

#include "engine.h"
#include "follower.h"
#include "hold.h"
#include "protocol.h"

using namespace dj;

namespace {

std::vector<int16_t> gBed(4 * 16 * kGrid, 0);
const int16_t kImpulse[1] = {16384};

struct Fire { int pad; uint64_t at; int64_t pos; };
std::vector<Fire> gFires;

// Records the musical position each pad fired at. Inside render(),
// now() - position() is the grid anchor, constant for the block.
void recordFire(void* ctx, int pad, uint64_t at) {
  auto* e = static_cast<Engine*>(ctx);
  const int64_t anchor = static_cast<int64_t>(e->now()) - e->position();
  gFires.push_back({pad, at, static_cast<int64_t>(at) - anchor});
}

Engine makeEngine() {
  Sample pads[kNumPads];
  for (auto& p : pads) p = {kImpulse, 1};
  return Engine({gBed.data(), gBed.size()}, pads);
}

uint32_t gRng = 0x9E3779B9;
uint32_t rnd(uint32_t n) {
  gRng ^= gRng << 13; gRng ^= gRng >> 17; gRng ^= gRng << 5;
  return gRng % n;
}

void run(Engine& e, size_t frames) {
  static int16_t buf[4096];
  while (frames) {
    const size_t n = frames < 4096 ? frames : 4096;
    e.render(buf, n);
    frames -= n;
  }
}

}  // namespace

void setUp() { gFires.clear(); }
void tearDown() {}

// ---- engine, follower mode -------------------------------------------------

void test_follow_start_sets_position() {
  Engine e = makeEngine();
  run(e, 5000);
  e.followStart(123456);
  TEST_ASSERT_TRUE(e.bedRunning());
  TEST_ASSERT_EQUAL_INT64(123456, e.position());
  run(e, 100);
  TEST_ASSERT_EQUAL_INT64(123556, e.position());
}

// Position larger than the engine's own clock: the anchor goes negative.
void test_follow_start_ahead_of_own_clock() {
  Engine e = makeEngine();
  e.followStart(10 * kGrid + 7);
  TEST_ASSERT_EQUAL_UINT64(kGrid - 7, e.nextGrid());
}

// A negative start position delays the bed; the bed audio begins exactly
// when position() reaches 0.
void test_follow_start_negative_delays_bed() {
  static int16_t ones[100];
  for (auto& x : ones) x = 10000;
  Sample pads[kNumPads];
  for (auto& p : pads) p = {kImpulse, 1};
  Engine e({ones, 100}, pads);
  e.followStart(-50);
  int16_t out[100];
  e.render(out, 100);
  for (int i = 0; i < 50; ++i) TEST_ASSERT_EQUAL_INT16(0, out[i]);
  for (int i = 50; i < 100; ++i) TEST_ASSERT_NOT_EQUAL(0, out[i]);
}

void test_schedule_fires_at_musical_position() {
  Engine e = makeEngine();
  e.onFire(recordFire, &e);
  run(e, 999);
  e.followStart(5 * kGrid + 100);
  TEST_ASSERT_TRUE(e.schedule(4, false, 7 * kGrid));
  run(e, 3 * kGrid);
  TEST_ASSERT_EQUAL(1, gFires.size());
  TEST_ASSERT_EQUAL(4, gFires[0].pad);
  TEST_ASSERT_EQUAL_INT64(7 * kGrid, gFires[0].pos);
}

// A press that arrives after its moment snaps to the next 16th.
void test_late_schedule_snaps_to_next_16th() {
  Engine e = makeEngine();
  e.onFire(recordFire, &e);
  e.followStart(10 * kGrid + 50);
  TEST_ASSERT_TRUE(e.schedule(1, false, 10 * kGrid));
  run(e, 2 * kGrid);
  TEST_ASSERT_EQUAL(1, gFires.size());
  TEST_ASSERT_EQUAL_INT64(11 * kGrid, gFires[0].pos);
}

void test_schedule_needs_running_bed() {
  Engine e = makeEngine();
  TEST_ASSERT_FALSE(e.schedule(0, false, 0));
}

void test_seek_keeps_queued_presses_on_their_beat() {
  Engine e = makeEngine();
  e.onFire(recordFire, &e);
  e.followStart(0);
  e.schedule(2, false, 4 * kGrid);
  run(e, kGrid);
  e.followSeek(300);    // jump ahead
  e.followSeek(-1000);  // and back further
  run(e, 5 * kGrid);
  TEST_ASSERT_EQUAL(1, gFires.size());
  TEST_ASSERT_EQUAL_INT64(4 * kGrid, gFires[0].pos);
}

void test_follower_bed_has_no_idle_timeout() {
  Engine e = makeEngine();
  e.followStart(0);
  run(e, kBedTimeout + 4096);
  TEST_ASSERT_TRUE(e.bedRunning());
  e.followStop();
  TEST_ASSERT_FALSE(e.bedRunning());
}

void test_press_reports_position_variant_and_session() {
  Engine e = makeEngine();
  static const int16_t kTwo[2] = {1, 1};
  e.setRepeatVariant(2, {kTwo, 2});
  run(e, 1234);
  Engine::Press p = e.press(2);
  TEST_ASSERT_TRUE(p.queued);
  TEST_ASSERT_FALSE(p.variant);
  TEST_ASSERT_EQUAL_INT64(kGrid, p.pos);
  TEST_ASSERT_EQUAL_UINT32(1, p.session);
  run(e, 100);
  p = e.press(2);
  TEST_ASSERT_TRUE(p.variant);
  TEST_ASSERT_EQUAL_INT64(kGrid, p.pos);
  run(e, kBedTimeout + 4096);             // bed stops
  p = e.press(0);
  TEST_ASSERT_EQUAL_UINT32(2, p.session);  // new bed run
}

void test_chime_silences_everything_else() {
  static int16_t ones[50];
  for (auto& x : ones) x = 8000;
  static int16_t bell[10];
  for (auto& x : bell) x = 4000;
  Sample pads[kNumPads];
  for (auto& p : pads) p = {ones, 50};
  Engine e({ones, 50}, pads);
  e.press(0);
  int16_t out[400];
  e.render(out, 400);                      // bed + pad sounding
  e.press(1);                              // queued, never plays
  e.chime({bell, 10});
  TEST_ASSERT_FALSE(e.bedRunning());
  e.render(out, 400);
  for (int i = 0; i < 10; ++i) TEST_ASSERT_NOT_EQUAL(0, out[i]);
  for (int i = 10; i < 400; ++i) TEST_ASSERT_EQUAL_INT16(0, out[i]);
}

// ---- lock loop -------------------------------------------------------------

void test_follower_large_error_seeks() {
  Follower f;
  TEST_ASSERT_EQUAL_INT64(5000, f.observe(5000));
  TEST_ASSERT_EQUAL_INT64(0, f.observe(10));
  TEST_ASSERT_EQUAL_UINT32(1, f.seeks());
}

void test_follower_small_error_slews_one_sample_at_a_time() {
  Follower f;
  f.observe(200);
  int total = 0;
  for (int i = 0; i < 2000; ++i) total += f.adjust();
  // Slews until inside the deadband, never past it.
  TEST_ASSERT_TRUE(total >= 200 - (int)Follower::kDeadband - 1 && total <= 200 - (int)Follower::kDeadband + 1);
  TEST_ASSERT_TRUE(std::fabs(f.error()) <= Follower::kDeadband);
}

// Two boards, clocks off by +-300 ppm, each with up to +-5 ms of jitter in
// its position estimate, packets every 100 ms. After settling, the follower
// stays within 3 ms of where it should be, and every press the leader makes
// sounds on the follower at the same musical position.
static void simulate(double ppm) {
  Engine lead = makeEngine();
  Engine fol = makeEngine();
  fol.onFire(recordFire, &fol);
  Follower f;
  gFires.clear();

  const double fs = kSampleRate;
  const double folRate = fs * (1 + ppm * 1e-6);   // follower's output rate
  const size_t kB = 256;
  double tLead = 0, tFol = 0;                     // next block start, seconds
  double nextPkt = 0.1, nextPress = 0.3;
  std::vector<Engine::Press> presses;
  bool started = false;
  double worst = 0;

  struct InFlight { double arrive; int64_t pos; bool press; Engine::Press p; };
  std::vector<InFlight> air;

  // The leader's exact position at time t. lead.position() is its render
  // head, which belongs to time tLead (the start of its next block).
  auto leadPos = [&](double t) { return lead.position() + (t - tLead) * fs; };
  auto jitter = [&]() { return (int64_t)rnd(221) - 110; };   // +-5 ms

  while (tFol < 120.0) {
    if (tLead <= tFol) {
      // Leader block. The first press starts its bed.
      if (tLead >= nextPress) {
        Engine::Press p = lead.press(rnd(kNumPads));
        presses.push_back(p);
        air.push_back({tLead + 0.002, lead.position() + jitter(), true, p});
        nextPress += 0.05 + rnd(400) / 1000.0;
      }
      if (lead.bedRunning() && tLead >= nextPkt) {
        air.push_back({tLead + 0.002, lead.position() + jitter(), false, {}});
        nextPkt += 0.1;
      }
      run(lead, kB);
      tLead += kB / fs;
      continue;
    }
    // Follower block: handle what has arrived, then lock and render.
    for (size_t i = 0; i < air.size();) {
      if (air[i].arrive > tFol) { ++i; continue; }
      const int64_t rxUs = (int64_t)(air[i].arrive * 1e6);
      const int64_t nowUs = (int64_t)(tFol * 1e6);
      const int64_t target = followTarget(air[i].pos, rxUs, nowUs);   // air time 2 ms = kAirUs
      if (!started) {
        fol.followStart(target);
        f.reset();
        started = true;
      } else {
        const int64_t seek = f.observe(target - fol.position() + jitter());
        if (seek) fol.followSeek(seek);
      }
      if (air[i].press) fol.schedule(0, false, air[i].p.pos);
      air.erase(air.begin() + i);
    }
    const int a = started ? f.adjust() : 0;
    run(fol, kB + a);
    tFol += kB / folRate;
    if (started && tFol > 20.0) {
      const double err = leadPos(tFol) - kFollowLag - fol.position();
      if (std::fabs(err) > worst) worst = std::fabs(err);
    }
  }
  printf("ppm %+.0f worst %.2f ms seeks %u slews %u fires %zu presses %zu\n", ppm, worst / fs * 1000, f.seeks(), f.slews(), gFires.size(), presses.size());
  TEST_ASSERT_TRUE_MESSAGE(worst < 0.003 * fs, "follower drifted more than 3 ms");
  TEST_ASSERT_EQUAL(0, f.seeks());
  // Every press played, each exactly at the leader's musical position.
  TEST_ASSERT_TRUE(gFires.size() >= presses.size() - 1);   // last may be in flight
  for (size_t i = 0; i < gFires.size(); ++i) TEST_ASSERT_EQUAL_INT64(presses[i].pos, gFires[i].pos);
}

void test_lock_with_follower_clock_fast() { simulate(+300); }
void test_lock_with_follower_clock_slow() { simulate(-300); }

// ---- wire format -----------------------------------------------------------

void test_decode_accepts_valid_and_rejects_junk() {
  Packet p = {};
  p.magic = kLinkMagic;
  p.type = kPress;
  p.pad = 8;
  Packet out;
  TEST_ASSERT_TRUE(decode(reinterpret_cast<uint8_t*>(&p), sizeof p, out));
  TEST_ASSERT_FALSE(decode(reinterpret_cast<uint8_t*>(&p), sizeof p - 1, out));
  p.pad = 9;
  TEST_ASSERT_FALSE(decode(reinterpret_cast<uint8_t*>(&p), sizeof p, out));
  p.pad = 0;
  p.magic ^= 1;
  TEST_ASSERT_FALSE(decode(reinterpret_cast<uint8_t*>(&p), sizeof p, out));
  p.magic = kLinkMagic;
  p.type = 99;
  TEST_ASSERT_FALSE(decode(reinterpret_cast<uint8_t*>(&p), sizeof p, out));
}

void test_follow_target_extrapolates_and_lags() {
  // Arrived 10 ms ago: the leader has moved on by 10 ms + air time.
  const int64_t t = followTarget(100000, 0, 10000);
  TEST_ASSERT_EQUAL_INT64(100000 + (10000 + kAirUs) * 22050 / 1000000 - kFollowLag, t);
}

// ---- long press ------------------------------------------------------------

void test_hold_fires_once_after_threshold_and_rearms() {
  Hold h(1000);
  TEST_ASSERT_FALSE(h.update(true, 0));
  TEST_ASSERT_FALSE(h.update(true, 999));
  TEST_ASSERT_TRUE(h.update(true, 1000));
  TEST_ASSERT_FALSE(h.update(true, 5000));
  TEST_ASSERT_FALSE(h.update(false, 5001));
  TEST_ASSERT_FALSE(h.update(true, 6000));
  TEST_ASSERT_FALSE(h.update(false, 6500));   // short press: nothing
  TEST_ASSERT_FALSE(h.update(true, 7000));
  TEST_ASSERT_TRUE(h.update(true, 8000));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_follow_start_sets_position);
  RUN_TEST(test_follow_start_ahead_of_own_clock);
  RUN_TEST(test_follow_start_negative_delays_bed);
  RUN_TEST(test_schedule_fires_at_musical_position);
  RUN_TEST(test_late_schedule_snaps_to_next_16th);
  RUN_TEST(test_schedule_needs_running_bed);
  RUN_TEST(test_seek_keeps_queued_presses_on_their_beat);
  RUN_TEST(test_follower_bed_has_no_idle_timeout);
  RUN_TEST(test_press_reports_position_variant_and_session);
  RUN_TEST(test_chime_silences_everything_else);
  RUN_TEST(test_follower_large_error_seeks);
  RUN_TEST(test_follower_small_error_slews_one_sample_at_a_time);
  RUN_TEST(test_lock_with_follower_clock_fast);
  RUN_TEST(test_lock_with_follower_clock_slow);
  RUN_TEST(test_decode_accepts_valid_and_rejects_junk);
  RUN_TEST(test_follow_target_extrapolates_and_lags);
  RUN_TEST(test_hold_fires_once_after_threshold_and_rearms);
  return UNITY_END();
}
