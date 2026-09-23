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
mapping rapid repeats to sample variants: a second press of the same pad
within 2 grid steps (242 ms) plays the pad's variant if it has one
(`Engine::setRepeatVariant`). Currently only pad 3: closed hat -> open hat.

---

## Sample set — DONE

House/techno, A minor, 124.016 BPM — the tempo is defined by the quantise
grid (16th = exactly 2667 samples) so the 4-bar loop is exactly 64 grid
steps and never drifts against the quantiser. Eleven files in `data/raw/`,
16-bit LE PCM mono 22050 Hz, 557 KB total. Generated by
`tools/make_samples.py` (writes `data/raw/` + `data/preview/` wavs).

| Pad | File | Row |
|---|---|---|
| — | 00_loop.raw (4 bars, loops seamlessly) | bed |
| 1 | 01_kick | amber |
| 2 | 02_clap | amber |
| 3 | 03_hat | amber |
| 4 | 04_stab_am (A C E) | cyan |
| 5 | 05_stab_c (C E G) | cyan |
| 6 | 06_stab_f (F A C) | cyan |
| 7 | 07_sub (A2 — 55 Hz is inaudible on both speakers) | magenta |
| 8 | 08_pluck (E4) | magenta |
| 9 | 09_riser | magenta |
| 3 (repeat) | 10_hat_open — plays when pad 3 is pressed again within 2 grid steps | variant |

Bar 4 of the loop drops the last kick and adds a clap flam so the bed has
a turnaround.

Reference implementation of mixer + quantiser: `tools/simulator.py` (Engine
class); the firmware port is `lib/engine/` and is verified sample-exact
against it. Grid = 2667 samples = 121 ms. Mixer: bed x0.80, one-shots x0.85,
then tanh(x * 0.62) — ceiling 1.0, so the sum can never hard-clip (1.15
clipped at bed + 2 aligned hits). Bed stops after 8 s without a press.

---

## Pin map — outside (ESP32-S3 N16R8, DevKitC-1 layout, all on header J1)

| GPIO | Function |
|---|---|
| 4, 5, 6, 7, 15, 16, 17, 18, 8 | Pads 1-9 (reading order), INPUT_PULLUP, active low |
| 10 | I2S DIN |
| 11 | I2S LRC / WS |
| 12 | I2S BCLK |
| 13 | WS2812 data (330R at pixel end) |

Never use: 33-37 (octal PSRAM), 19/20 (USB), 43/44 (UART0),
0/3/45/46 (strapping), 38/48 (onboard RGB LED).

Pixel chain order = pad order 1-9 (reading order), so pixel index == pad index.
Cap brightness at ~40% — power budget and PTC hold current depend on it.

## Pin map — inside (ESP32-WROOM-32D)

| GPIO | Function |
|---|---|
| 26 | I2S BCLK |
| 25 | I2S LRC / WS |
| 22 | I2S DIN |
| 34 | Volume pot wiper (ADC1 — ADC2 is unusable with WiFi on). Pot ends on 3V3/GND. |

Never use: 6-11 (flash), 12 (pulled high at boot = 1.8V flash, won't boot).

## Power

Outside: 12V pair -> 500mA PTC -> SS34 series -> SMAJ15A + 470uF 25V to GND
-> MP1584 (5.0V) -> SS34 series -> ~4.6V rail -> S3 5V pin, MAX98357A VIN,
pixels (+1000uF 10V at pad 1).
Inside: 230V -> IRM-10-12 -> 12V -> (pair to door) and (MP1584 5.0V -> SS34
-> ~4.6V rail -> WROOM 5V, MAX98357A VIN).

The series SS34 after each buck makes USB-while-powered safe and drops pixel
VDD so 3.3V data clears the 0.7*VDD threshold.

## Amp config

- SD pin unconnected on both. Firmware writes identical L and R samples.
- GAIN: outside tied to VIN (6 dB, quiet feedback); inside floating (9 dB).
- Bridge-tied output: no speaker lead to ground, ever.

## Build config

Samples are embedded via `board_build.embed_files` (memory-mapped, zero copy),
not LittleFS. Symbols: `_binary_data_raw_00_loop_raw_start` / `_end`.

```ini
[base]
platform = https://github.com/pioarduino/platform-espressif32/releases/download/stable/platform-espressif32.zip
framework = arduino
monitor_speed = 115200
board_build.embed_files =
  data/raw/00_loop.raw
  data/raw/01_kick.raw
  data/raw/02_clap.raw
  data/raw/03_hat.raw
  data/raw/04_stab_am.raw
  data/raw/05_stab_c.raw
  data/raw/06_stab_f.raw
  data/raw/07_sub.raw
  data/raw/08_pluck.raw
  data/raw/09_riser.raw
  data/raw/10_hat_open.raw

[env:outside]
extends = base
board = esp32-s3-devkitc-1
board_upload.flash_size = 16MB
board_build.partitions = default_16MB.csv
board_build.arduino.memory_type = qio_opi
build_flags = -DUNIT_OUTSIDE -DBOARD_HAS_PSRAM
lib_deps = adafruit/Adafruit NeoPixel

[env:inside]
extends = base
board = esp32dev
board_build.partitions = huge_app.csv
build_flags = -DUNIT_INSIDE
```

WROOM-32D needs huge_app.csv — default gives the app 1.25 MB, too small for
firmware + 557 KB of samples.

Bench phases 1-5 run on USB power only. The full bring-up sequence with pass
criteria lives in the build guide.

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

- Whether conduit exists for a Cat6 pull.
- Idle LED behaviour: dark, dim breathing, or a slow colour drift.

---

## Environment

- Ender 3V2 available, plus friends with modern printers
- Experienced developer and DIY electronics builder — no need to explain basics
- Wants precise, well-researched answers without hedging
