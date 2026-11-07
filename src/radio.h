// WiFi, ESP-NOW and OTA for both units.
//
// Both units join the home WiFi when include/secrets.h exists. Being on the
// same router puts them on the same channel automatically, even if the
// router moves channel by itself. ESP-NOW runs alongside on that channel.
//
// Outside: finds the inside unit by broadcasting kHello until it answers,
// then sends unicast (MAC-level retries, plus up to two resends of a press or
// chime whose retries all failed). If the outside unit cannot reach the
// router within 20 s of boot it drops WiFi and, while the inside unit is not
// answering, hops channels 1-13 looking for it; every 2 min while the pad is
// idle it tries the router again (a power cut brings the unit up minutes
// before the router).
//
// Inside: answers kHello, hands every packet to the callback.
#pragma once

#include <cstdint>

#include "protocol.h"

namespace radio {

// Runs on the WiFi task: copy the packet into a queue and return.
using RxFn = void (*)(const dj::Packet& p, int64_t rxUs);

void begin(const char* hostname, RxFn onRx);
void loop();                                    // from loop(): WiFi, OTA, discovery, resends
bool send(const dj::Packet& p, bool reliable);  // outside -> inside; false if no peer yet
bool linked();                                  // the other unit was heard in the last 3 s
void setBusy(bool playing);                     // outside: hold WiFi retries while the bed plays
bool otaActive();                               // firmware update writing flash: mute audio
void printStatus();

}  // namespace radio
