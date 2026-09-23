// Sample set embedded in flash via board_build.embed_files (memory-mapped,
// zero copy). Order and names match CLAUDE.md / tools/make_samples.py.
#pragma once

#include "engine.h"

namespace samples {

extern const dj::Sample kBed;                 // 00_loop — 4 bars, loops seamlessly
extern const dj::Sample kPads[dj::kNumPads];  // 01_kick .. 09_riser, pad order
extern const dj::Sample kHatOpen;             // 10_hat_open — pad 3 repeat variant

// Pad indices are zero-based: pad 4 in CLAUDE.md (04_stab_am) is kPads[3].
constexpr int kStabAm = 3;
constexpr int kHat    = 2;

// Halts with a log message if any sample is not 2-byte aligned. objcopy only
// guarantees byte alignment for embedded files, and an int16 load from an odd
// flash address is a LoadStoreAlignment fault on Xtensa.
void verify();

}  // namespace samples
