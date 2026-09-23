// Host tests for the engine clock and quantiser.  pio test -e native
#include <unity.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <vector>

#include "engine.h"

using namespace dj;

namespace {

// Silent bed (so the only non-zero output is one-shots) and a one-sample
// impulse per pad, so output[t] != 0 iff a pad fired at sample t.
std::vector<int16_t> gBed(4 * 16 * kGrid, 0);
const int16_t kImpulse[1] = {16384};

struct Fire { int pad; uint64_t at; };
std::vector<Fire> gFires;

void recordFire(void*, int pad, uint64_t at) { gFires.push_back({pad, at}); }

Engine makeEngine() {
  Sample pads[kNumPads];
  for (auto& p : pads) p = {kImpulse, 1};
  Engine e({gBed.data(), gBed.size()}, pads);
  e.setQuantise(true);
  e.onFire(recordFire, nullptr);
  gFires.clear();
  return e;
}

// Deterministic PRNG so failures reproduce.
uint32_t gRng = 0x2545F491;
uint32_t rnd(uint32_t n) {
  gRng ^= gRng << 13; gRng ^= gRng >> 17; gRng ^= gRng << 5;
  return gRng % n;
}

// Advance the clock by `frames`, rendering in arbitrary block sizes, and
// append every output sample to `out`.
void advance(Engine& e, uint64_t frames, std::vector<int16_t>& out) {
  while (frames) {
    const size_t n = std::min<uint64_t>(frames, 1 + rnd(512));
    const size_t base = out.size();
    out.resize(base + n);
    e.render(out.data() + base, n);
    frames -= n;
  }
}

// Smallest 16th boundary strictly after `t` on a grid anchored at `anchor`.
uint64_t expectedBoundary(uint64_t anchor, uint64_t t) {
  return anchor + ((t - anchor) / kGrid + 1) * kGrid;
}

}  // namespace

void setUp() {}
void tearDown() {}

void test_presses_at_arbitrary_offsets_land_on_16th_boundaries() {
  Engine e = makeEngine();
  std::vector<int16_t> out;
  std::vector<Fire> expected;

  advance(e, 12345, out);          // arbitrary start, not a multiple of anything
  const uint64_t anchor = e.now();

  for (int i = 0; i < 500; ++i) {
    const uint64_t t = e.now();
    const int pad = rnd(kNumPads);
    e.press(pad);
    TEST_ASSERT_TRUE(e.bedRunning());
    expected.push_back({pad, expectedBoundary(anchor, t)});
    advance(e, rnd(3 * kGrid), out);   // < 8 s, so the bed never times out
  }
  advance(e, 2 * kGrid, out);          // flush the last pending press

  TEST_ASSERT_EQUAL(expected.size(), gFires.size());
  for (size_t i = 0; i < expected.size(); ++i) {
    TEST_ASSERT_EQUAL(expected[i].pad, gFires[i].pad);
    TEST_ASSERT_EQUAL_UINT64(expected[i].at, gFires[i].at);
    TEST_ASSERT_EQUAL_UINT64(0, (gFires[i].at - anchor) % kGrid);
  }

  // The audio agrees with the callback: sound only on fire samples.
  std::vector<bool> isFire(out.size(), false);
  for (const Fire& f : gFires) isFire[f.at] = true;
  for (uint64_t s = 0; s < out.size(); ++s) {
    if (isFire[s] != (out[s] != 0)) {
      char msg[64];
      snprintf(msg, sizeof msg, "sample %llu", (unsigned long long)s);
      TEST_FAIL_MESSAGE(msg);
    }
  }
}

void test_grid_is_2667_samples() {
  Engine e = makeEngine();
  std::vector<int16_t> out;
  e.press(0);                         // anchors grid at 0
  TEST_ASSERT_EQUAL_UINT64(kGrid, e.nextGrid());
  advance(e, 1, out);
  TEST_ASSERT_EQUAL_UINT64(2667, e.nextGrid());
  advance(e, 2665, out);              // now = 2666
  TEST_ASSERT_EQUAL_UINT64(2667, e.nextGrid());
}

void test_press_on_a_boundary_waits_for_the_next_one() {
  Engine e = makeEngine();
  std::vector<int16_t> out;
  e.press(0);
  advance(e, kGrid, out);             // now == first boundary, already played
  TEST_ASSERT_EQUAL_UINT64(2 * kGrid, e.nextGrid());
}

void test_quantise_off_plays_immediately() {
  Engine e = makeEngine();
  e.setQuantise(false);
  std::vector<int16_t> out;
  advance(e, 1000, out);
  e.press(4);
  advance(e, 10, out);
  TEST_ASSERT_EQUAL(1, gFires.size());
  TEST_ASSERT_EQUAL_UINT64(1000, gFires[0].at);
}

void test_bed_times_out_and_next_press_reanchors_grid() {
  Engine e = makeEngine();
  std::vector<int16_t> out;
  e.press(0);
  advance(e, kBedTimeout + 1024, out);
  TEST_ASSERT_FALSE(e.bedRunning());

  advance(e, 777, out);
  const uint64_t anchor = e.now();
  e.press(1);
  TEST_ASSERT_TRUE(e.bedRunning());
  advance(e, 2 * kGrid, out);
  TEST_ASSERT_EQUAL_UINT64(anchor + kGrid, gFires.back().at);
}

void test_simultaneous_presses_fire_on_the_same_sample() {
  Engine e = makeEngine();
  std::vector<int16_t> out;
  e.press(0);
  advance(e, 100, out);
  for (int p = 0; p < kNumPads; ++p) e.press(p);
  advance(e, 2 * kGrid, out);
  TEST_ASSERT_EQUAL(10, gFires.size());
  for (size_t i = 1; i < gFires.size(); ++i) TEST_ASSERT_EQUAL_UINT64(kGrid, gFires[i].at);
}

// Pad 2 has a two-sample variant; a repeat inside kRepeatWindow plays it
// (output 2 samples long), a slower repeat plays the base (1 sample).
void test_rapid_repeat_plays_variant() {
  Engine e = makeEngine();
  static const int16_t kTwo[2] = {16384, 16384};
  e.setRepeatVariant(2, {kTwo, 2});
  std::vector<int16_t> out;

  e.press(2);                                   // first: base
  advance(e, kGrid + 10, out);
  e.press(2);                                   // ~1 grid later: variant
  advance(e, kGrid + 10, out);
  e.press(2);                                   // still inside window: variant
  advance(e, 3 * kGrid, out);
  e.press(2);                                   // >2 grids since last: base
  advance(e, 2 * kGrid, out);

  TEST_ASSERT_EQUAL(4, gFires.size());
  auto lenAt = [&](uint64_t at) { int n = 0; while (out[at + n] != 0) ++n; return n; };
  TEST_ASSERT_EQUAL(1, lenAt(gFires[0].at));
  TEST_ASSERT_EQUAL(2, lenAt(gFires[1].at));
  TEST_ASSERT_EQUAL(2, lenAt(gFires[2].at));
  TEST_ASSERT_EQUAL(1, lenAt(gFires[3].at));
}

void test_output_never_clips() {
  Engine e = makeEngine();
  static int16_t loud[4000];
  for (auto& x : loud) x = 32767;
  Sample pads[kNumPads];
  for (auto& p : pads) p = {loud, 4000};
  Engine hot({loud, 4000}, pads);
  int16_t buf[4000];
  for (int p = 0; p < kNumPads; ++p) hot.press(p);   // bed + 9 full-scale shots
  hot.render(buf, 4000);
  int16_t peak = 0;
  for (int16_t x : buf) if (x > peak) peak = x;
  TEST_ASSERT_TRUE(peak >= 32000 && peak <= 32767);  // saturates, never wraps
  (void)e;
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_presses_at_arbitrary_offsets_land_on_16th_boundaries);
  RUN_TEST(test_grid_is_2667_samples);
  RUN_TEST(test_press_on_a_boundary_waits_for_the_next_one);
  RUN_TEST(test_quantise_off_plays_immediately);
  RUN_TEST(test_bed_times_out_and_next_press_reanchors_grid);
  RUN_TEST(test_simultaneous_presses_fire_on_the_same_sample);
  RUN_TEST(test_rapid_repeat_plays_variant);
  RUN_TEST(test_output_never_clips);
  return UNITY_END();
}
