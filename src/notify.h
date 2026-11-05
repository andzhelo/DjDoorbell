// Phone notification (outside unit) through ntfy (https://ntfy.sh): the
// phone's ntfy app subscribes to a topic, the unit POSTs a message to that
// topic. No account; the topic name is the only secret, so use a long
// random one. Sending happens on its own task (a TLS handshake takes a
// second or two) so the pads, pixels and link never wait for it.
#pragma once

#include <cstddef>
#include <cstdint>

namespace notify {

struct Settings {
  bool on;
  char server[64];   // "https://ntfy.sh" or a self-hosted one
  char topic[64];    // [A-Za-z0-9_-], 1..63 chars
};

void begin();                                   // load settings, start the sender task
const Settings& settings();
bool set(const Settings& s);                    // validate + save
bool send(const char* title, const char* message, const char* tags);   // queued; false if off/full
bool lastOk();                                  // result of the last attempt
void toJson(char* buf, size_t len);

}  // namespace notify
