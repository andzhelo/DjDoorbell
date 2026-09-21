# DJ Doorbell — project spec

A doorbell where the visitor performs instead of pressing a button. A 3x3 pad
of arcade buttons outside the apartment door; the performance plays through a
speaker inside. Presses are quantised to a 16th-note grid over a running beat
bed, so a stranger mashing buttons still produces something musical.

Apartment in a multi-floor building, Israel. Indoor hallway — no weather
exposure. Neighbours in a shared stairwell are the main real-world constraint.

---

## Hardware (ordered, ~2 weeks out)

### Outside unit — the pad
| Part | Notes |
|---|---|
| ESP32-S3 DevKitC-1 N16R8 | 16MB flash, 8MB PSRAM. Owns the master clock. |
| 9x 24mm arcade buttons, clear/frosted white cap | Sanwa-clone snap-in. Lamp holder twists out. |
| 9x WS2812B segments | Cut from a 30/m IP30 strip already owned. Sits in the vacated lamp cavity, facing the cap. |
| MAX98357A I2S amp | |
| 23/24mm 4ohm 2W speaker | Low-volume feedback only — the visitor must hear what they play. |
| MP1584EN buck | 12V -> 5V at the panel |
| 330R resistor | Data line into first pixel (already owned) |
| 1000uF cap | Across 5V/GND at the LED power injection point |

### Inside unit — the chime
| Part | Notes |
|---|---|
| ESP32-WROOM-32D | Already owned. 4MB flash, ~2MB usable for samples (~45s at 22.05kHz). Enough. |
| Mean Well IRM-10-12 | 230V -> 12V. Replaces the doorbell transformer. |
| MAX98357A I2S amp | |
| 3" 4ohm 5W full-range speaker | |
| 10k pot | Volume |

Reuses the existing chime enclosure — it is already mounted and already rated
for the mains entering it. Do not 3D-print a replacement for this one.

### Protection (outside unit, on the wire from the wall)
SS34 Schottky (reverse polarity), SMAJ15A TVS, 500mA polyfuse, 470uF bulk.

---

## Wall survey — confirmed by measurement

- Inside chime box has **230V mains** entering it (embossed on the housing).
  The old transformer secondary measured **8.18V AC**.
- **Exactly 2 conductors** run through the wall to the outside button, both
  brown. Everything else at the button was a jumper looping inside the module.
- No conduit confirmed. Worth one more flashlight check during install — if
  Cat6 can be pulled, take it and drop ESP-NOW entirely.
- Existing outside button is a Nisko illuminated bell push, 12-24V AC/DC, with
  a permanently-lit blue locator LED.

**Consequence:** 12V DC on the pair, ESP-NOW for pad events. Do not run the
project off the old 8V transformer — it is rated for intermittent coil pulses,
not continuous draw.

Rejected: analog audio superimposed on the pair. Unshielded bell wire running
near 230V would pick up 50Hz hum.

---

## Wiring

### Buttons — one GPIO each, NOT a matrix
Nine switches, nine GPIOs. COM to ground, NO to GPIO with internal pull-up.

This reverses an earlier decision. A 3x3 matrix without diodes ghosts on 3+
simultaneous presses, and simultaneous presses are the entire point of this
device. The S3 has GPIO to spare; direct wiring costs only wire.

### Pixels — one shared chain
Board data pin -> 330R -> pad 1 DIN. Pad 1 DOUT -> pad 2 DIN, and so on
through nine. 5V and GND are shared rails. One GPIO total.

Order the chain to follow grid reading order (top-left to bottom-right) so
firmware indices match physical position.

### Panel geometry
- 9 buttons, 24mm diameter, 30mm pitch -> 100x100mm faceplate
- Snap tabs expect 2-3mm panel thickness at the hole; add a recess if printing thicker
- Build a shell BEHIND the faceplate (100x100x25mm) rather than cramming
  electronics into the shallow Nisko wall box. The wall box becomes wire entry only.
- Measure real buttons with calipers before modelling: body diameter, snap-tab
  thickness, flange width, depth behind panel.
- Print a single-hole test coupon before the full plate.

---

## Firmware architecture

### Audio format
Raw 16-bit PCM mono at 22.05kHz, stored in flash. **Not MP3** — decode eats a
core and makes polyphony painful. Mixing is integer addition on int16 buffers.
~44KB per second of audio.

### Voices
- 1 loop voice — 2 or 4 bar drum/bass bed, runs continuously
- 9 one-shot voices — retriggerable, summed on top
- Divide each voice by 4 before summing, or nine simultaneous hits clip

### Clock
Derive from the I2S sample counter. **Never millis()** — it drifts audibly.

### Quantisation
Snap presses to the nearest 16th note. At 120 BPM that is a 125ms grid. Adds
slight latency; it is the difference between noise and music when a stranger
mashes buttons. Make it a compile-time toggle for A/B testing.

### Two-board sync
Both units store the same sample set and render audio locally. Only tiny event
packets cross the gap: pad index + grid phase. Outside unit owns the master
clock and broadcasts phase every bar; inside unit locks to it. Because both
quantise to the same grid, a dropped packet does not break sync.

### Sample set
Nine one-shots plus one loop. All melodic content in **A minor** so random
button-mashing stays consonant. Row colour encodes category:
- Row 1 (amber): kick, snare, hat
- Row 2 (cyan): three tuned stabs
- Row 3 (magenta): clap, blip, sweep

No velocity sensitivity — arcade microswitches are binary. Fake dynamics by
mapping rapid repeats to slightly varied sample variants.

---

## Features

Confirmed, all zero-cost:
- **Performance replay.** Record events (pad index + grid position), not audio.
  A whole performance is under 100 bytes. Inside unit replays from the same
  samples. Gives every visitor a signature.
- **Quiet hours.** Inside unit joins WiFi, gets NTP time, tells outside to cut
  volume after 22:00. This is the feature that keeps neighbours friendly.
- **Phone notification.** MQTT or Home Assistant webhook from the inside unit.
- **Plain-chime escape hatch.** Long-press the centre pad plays a normal
  2-second chime. Delivery drivers and plumbers want one button and to leave.

---

## Build order

1. **Sound design** (now, while parts ship) — nine samples + loop, A minor,
   converted to raw PCM. No hardware dependency. Deliverable: a folder of
   .raw files.
2. **Bench firmware** — WROOM-32D, one button, one amp. I2S out plus
   sample-accurate clock. Deliverable: one button plays in time with a loop.
3. **Link test — GATE.** Both boards on breadboards, one AT THE DOOR, one
   inside, door closed. Ten minutes of ESP-NOW packet loss measurement.
   If this fails, everything downstream changes. Test before printing or soldering.
4. **Full breadboard** — nine buttons, LED chain, both units talking. Tune the
   quantiser and volume here.
5. **Enclosure** — calipers, test coupon, faceplate, shell.
6. **Solder and assemble** — protection components go in before anything
   touches the wall.
7. **Install — BREAKER OFF.** Remove chime and transformer, IRM-10-12 in its
   place. Verify 12V on the pair with a meter before connecting the outside unit.
8. **Live tuning** — volume, quiet hours, LED brightness. Expect a week.

Solder all nine pixels as a chain on the bench and run a test pattern BEFORE
seating any of them in a button. Finding a bad joint after mounting is misery.

---

## Open decisions

- **Sound direction.** Current demo is house/techno. Alternatives: lo-fi
  (warmer, less startling at night), 8-bit (playful, reads as a toy), or
  organic — marimba/kalimba/hand percussion (most neighbour-friendly, ages best).
  Not yet chosen.
- Whether conduit exists for a Cat6 pull.
- Idle LED behaviour: dark, dim breathing, or a slow colour drift.

---

## Environment

- Ender 3V2 available, plus friends with modern printers
- Experienced developer and DIY electronics builder — no need to explain basics
- Wants precise, well-researched answers without hedging
