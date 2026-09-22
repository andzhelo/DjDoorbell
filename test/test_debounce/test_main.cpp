// Host tests for the pad debouncer.  pio test -e native
#include <unity.h>

#include <initializer_list>

#include "debounce.h"

using dj::Debouncer;

void setUp() {}
void tearDown() {}

// Feed `level` for `ms` milliseconds at 1 ms polling; count accepted presses.
static int hold(Debouncer& d, uint32_t& t, bool level, uint32_t ms) {
  int presses = 0;
  for (uint32_t i = 0; i < ms; ++i) presses += d.update(level, t++);
  return presses;
}

void test_clean_press_accepted_after_5ms() {
  Debouncer d(5);
  uint32_t t = 100;
  TEST_ASSERT_EQUAL(0, hold(d, t, true, 5));   // held for 0..4 ms
  TEST_ASSERT_TRUE(d.update(true, t++));       // 5 ms: accepted
  TEST_ASSERT_TRUE(d.down());
  TEST_ASSERT_EQUAL(0, hold(d, t, true, 500)); // holding doesn't repeat
}

void test_bounce_gives_one_press() {
  Debouncer d(5);
  uint32_t t = 0;
  int presses = 0;
  // Microswitch make: 3 ms of chatter, then solid.
  for (bool b : {true, false, true, true, false, true}) presses += d.update(b, t++);
  presses += hold(d, t, true, 20);
  // Release, with chatter too.
  for (bool b : {false, true, false, false, true, false}) presses += d.update(b, t++);
  presses += hold(d, t, false, 20);
  TEST_ASSERT_EQUAL(1, presses);
  TEST_ASSERT_FALSE(d.down());
}

void test_spike_shorter_than_5ms_is_ignored() {
  Debouncer d(5);
  uint32_t t = 0;
  TEST_ASSERT_EQUAL(0, hold(d, t, true, 4));
  TEST_ASSERT_EQUAL(0, hold(d, t, false, 50));
  TEST_ASSERT_FALSE(d.down());
}

void test_fast_repeats_each_count() {
  Debouncer d(5);
  uint32_t t = 0;
  int presses = 0;
  for (int i = 0; i < 10; ++i) {   // 10 taps at ~50 ms each: faster than any finger
    presses += hold(d, t, true, 25);
    presses += hold(d, t, false, 25);
  }
  TEST_ASSERT_EQUAL(10, presses);
}

void test_millis_wraparound() {
  Debouncer d(5);
  uint32_t t = 0xFFFFFFFE;
  TEST_ASSERT_EQUAL(1, hold(d, t, true, 10));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_clean_press_accepted_after_5ms);
  RUN_TEST(test_bounce_gives_one_press);
  RUN_TEST(test_spike_shorter_than_5ms_is_ignored);
  RUN_TEST(test_fast_repeats_each_count);
  RUN_TEST(test_millis_wraparound);
  return UNITY_END();
}
