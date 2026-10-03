// Outside -> inside packets over ESP-NOW. Small, fixed size, one struct.
//
// Every packet carries the leader's bed state (session, running, position at
// the moment of sending), so any packet that arrives can start or correct the
// follower; none of them is special. Presses and chimes go unicast with MAC
// retries; the outside resends a press once more if the retries all fail.
//
// No hardware calls: host-testable.
#pragma once

#include <cstdint>
#include <cstring>

#include "engine.h"

namespace dj {

constexpr uint32_t kLinkMagic = 0xD1D0B312;

enum PacketType : uint8_t {
  kHello    = 1,   // outside, broadcast: "inside unit, where are you?"
  kHelloAck = 2,   // inside -> outside: "here"
  kSync     = 3,   // outside: bed state, 10x a second while playing, 1x idle
  kPress    = 4,   // outside: pad `pad` sounds at musical position `at`
  kChime    = 5,   // outside: long press, play the plain chime
};

enum PacketFlags : uint8_t {
  kFlagRunning = 1 << 0,   // the leader's bed is playing
  kFlagVariant = 1 << 1,   // kPress: play the pad's repeat variant
};

struct __attribute__((packed)) Packet {
  uint32_t magic;
  uint8_t  type;
  uint8_t  pad;        // kPress: 0-8
  uint8_t  flags;
  uint8_t  reserved;
  uint16_t seq;        // kPress / kChime: drops duplicates
  uint32_t session;    // leader's bed run; changes on every bed start
  int64_t  pos;        // leader's musical position when this was sent, samples
  int64_t  at;         // kPress: musical position the pad sounds at
};
static_assert(sizeof(Packet) == 30, "wire format");

inline bool decode(const uint8_t* data, int len, Packet& out) {
  if (len != static_cast<int>(sizeof(Packet))) return false;
  memcpy(&out, data, sizeof out);
  return out.magic == kLinkMagic && out.type >= kHello && out.type <= kChime &&
         (out.type != kPress || out.pad < kNumPads);
}

// The inside unit plays this far behind the outside one. A press is sent the
// moment it is pressed and sounds at the next 16th at the earliest, so with
// this lag it reaches the inside before it is due even when it arrives
// late in the inside's render block (11.6 ms) after a few ms in the air.
// 30 ms between two speakers on opposite sides of a door is heard as one.
constexpr int64_t kFollowLag = kSampleRate * 30 / 1000;   // 661 samples
// One-way air time for a 30-byte ESP-NOW frame plus stack latency, roughly
// half the 4.5 ms round trip measured in the link test.
constexpr int64_t kAirUs = 2000;

// Where the follower should be at time `nowUs`, given a packet stamped with
// the leader's position `pos` that arrived at `rxUs`.
inline int64_t followTarget(int64_t pos, int64_t rxUs, int64_t nowUs) {
  return pos + (nowUs - rxUs + kAirUs) * kSampleRate / 1000000 - kFollowLag;
}

}  // namespace dj
