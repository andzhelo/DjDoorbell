#include "samples.h"

#include <Arduino.h>

#define EMBEDDED(name)                                                  \
  extern "C" const uint8_t _binary_data_raw_##name##_raw_start[];      \
  extern "C" const uint8_t _binary_data_raw_##name##_raw_end[];

#define SAMPLE(name)                                                             \
  dj::Sample {                                                                   \
    reinterpret_cast<const int16_t*>(_binary_data_raw_##name##_raw_start),       \
    static_cast<size_t>(_binary_data_raw_##name##_raw_end -                      \
                        _binary_data_raw_##name##_raw_start) / sizeof(int16_t)   \
  }

EMBEDDED(00_loop)
EMBEDDED(01_kick)
EMBEDDED(02_clap)
EMBEDDED(03_hat)
EMBEDDED(04_stab_am)
EMBEDDED(05_stab_c)
EMBEDDED(06_stab_f)
EMBEDDED(07_sub)
EMBEDDED(08_pluck)
EMBEDDED(09_riser)
EMBEDDED(10_hat_open)

namespace samples {

const dj::Sample kBed = SAMPLE(00_loop);

const dj::Sample kPads[dj::kNumPads] = {
  SAMPLE(01_kick),    SAMPLE(02_clap),  SAMPLE(03_hat),
  SAMPLE(04_stab_am), SAMPLE(05_stab_c), SAMPLE(06_stab_f),
  SAMPLE(07_sub),     SAMPLE(08_pluck), SAMPLE(09_riser),
};

const dj::Sample kHatOpen = SAMPLE(10_hat_open);

void verify() {
  size_t bytes = kBed.len * 2 + kHatOpen.len * 2;
  bool ok = (reinterpret_cast<uintptr_t>(kBed.data) & 1) == 0 &&
            (reinterpret_cast<uintptr_t>(kHatOpen.data) & 1) == 0;
  for (const dj::Sample& s : kPads) {
    bytes += s.len * 2;
    ok &= (reinterpret_cast<uintptr_t>(s.data) & 1) == 0;
  }
  Serial.printf("Samples:  %u bytes, bed @ %p\n", (unsigned)bytes, kBed.data);
  if (ok) return;
  Serial.println("FATAL: embedded sample at odd address — int16 reads would fault");
  for (;;) delay(1000);
}

}  // namespace samples
