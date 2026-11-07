# DJ Doorbell — project spec

A doorbell where the visitor performs instead of pressing a button. A 3x3 pad
of arcade buttons outside the apartment door; the performance plays through a
speaker inside. Presses are quantised to a 16th-note grid over a running beat
bed, so a stranger mashing buttons still produces something musical.

Apartment in a multi-floor building, Israel. Indoor hallway — no weather
exposure. Neighbours in a shared stairwell are the main real-world constraint.

---

## Hardware (all arrived, 2026-10-02)

### Outside unit — the pad
| Part | Notes |
|---|---|
| ESP32-S3 DevKitC-1 N16R8 | 16MB flash, 8MB PSRAM. Owns the master clock. |
| 9x 24mm arcade buttons, XW-OBSC (OBSC-24 clone) | Frosted plunger, clear body, microswitch moulded into the base (not removable), 2.8 mm tabs. No lamp holder. Fits a 24.0 mm hole in a 2.5 mm panel. |
| 3x WS2812B strip pieces, 3 LEDs each | Uncut 100 mm pieces of the 30/m IP30 strip already owned. Side-light each row — see Pixels. |
| MAX98357A I2S amp | |
| 40mm 4ohm 2W speaker | Low-volume feedback only — the visitor must hear what they play. |
| MP1584EN buck | 12V -> 5V at the panel. These boards are reversed: trimmer CLOCKWISE = LOWER voltage. Adjust with nothing on the output. |
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

The old chime enclosure keeps the IRM-10-12 (mains stays there). The inside
unit itself was too big for it, so it gets its own printed box: `cad/inside_*.scad`
(130 x 126 x 46 mm landscape layout 2026-11-02). PRINTED AND MOUNTED 2026-11-05:
inside unit on wall power in its box, volume knob (9 mm RV09 pot on GPIO34)
working, `inside` env now builds with `-DDJ_VOLUME_POT=1`. BOTH UNITS INSTALLED.

### Protection (outside unit, on the wire from the wall)
**Simplified 2026-10-06:** fuse (inline holder) + 470uF across the buck input,
on a small perfboard. The SS34s and SMAJ15A were dropped: the owner could not
reliably hand-solder the SMD parts on perfboard, and none is critical. Costs:
no reverse-polarity protection (measure the pair's polarity before connecting
at install) and no USB/wall back-feed protection (never plug USB into the S3
while wall-powered; use OTA). The LED strip keeps its own SS34 on its +5V lead
(needed: 3.3 V data vs 5 V VDD). Fuse must be 500mA; the bag is marked
0313001, which is 1A — check the cap before fitting.

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

### Pixels — one shared chain, side-lit
The switch block covers almost the whole button base, so LEDs can't go under
or inside the buttons. Each row is lit from the side (hand-tested: the cap
glows): one uncut 3-LED strip piece stands on edge in a channel on the
faceplate back, just below its row, LEDs facing up into the clear bodies.

Board data pin -> 330R -> row 1 strip DIN, left to right. Row 1 DOUT (right
end) -> wire back to row 2 DIN (left end), same for row 3. Arrows on all
three strips point left to right. 5V and GND shared. One GPIO total.
As built, rows 1 and 3 ended up swapped end for end in the chain; firmware
maps pad -> LED with `kPixelOf` in src/main.cpp (no re-solder needed).

### Enclosure — `cad/` (parametric OpenSCAD; see cad/README.md)
- Faceplate 114x145x2.5mm: 3x3 holes Ø24.0 on 30mm pitch (from the printed
  coupon), 3 LED strip channels on the back, 40mm speaker ring + grille in a
  45mm bay below the pad. 114 wide because a 3-LED strip piece is 100mm.
- Shell 114x145x34mm behind it; the Nisko wall box is wire entry only.
  M3 heat-set inserts + 4x M3x8 countersunk screws.
- Shell is printed. Faceplate printed once, but must be REPRINTED: the
  speaker ring/grille changed from 23.5mm to 40mm after the speaker arrived.
- `cad/render.sh` exports STLs; `assembly.scad` has interference checks.
- Boards have no mounting holes: foam tape on the shell floor, under the
  speaker.

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

Pixel chain order as built: pad -> LED via `kPixelOf` in src/main.cpp
(1<->9, 2<->8, 3<->7 swapped; middle row in order).
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

Outside: 12V pair -> 500mA fuse -> 470uF 25V to GND -> MP1584 (5.0V, set
and verified) -> 5V rail -> S3 5V pin, MAX98357A VIN, pixels via the SS34 on
the strip lead (+1000uF 10V at pad 1).
2026-10-10: turning the trimmer the wrong way while the S3 was connected likely
over-volted the 5V rail; S3 then failed flash writes (boot: partition table
error). Amp and pixels on that rail are suspect too — check before reuse.
Confirmed: that S3 (MAC 14:c1:9f:51:7b:1c) reads flash but every erase fails
(C400, both USB sockets; 3V3 rail fine) — dead. Replacement N16R8 (MAC 44:BD:8D:68:03:A0) flashed with `outside` on 2026-10-19: boots, WiFi + OTA up.
2026-10-19 bench on that board: pads, I2S and pixel data all verified at the
board; outside MAX98357A confirmed DEAD (inside unit's spare plays on the same
wires); installed LED chain still unresponsive (first strip / data wire
suspect, faceplate must come off to check). 2026-10-20: a spare-strip test
powered from the 3V3 pin shorted and smoked the on-board 3.3 V regulator; the
board brownout-loops on USB power. CORRECTED 2026-10-20: the chip and the 3.3 V
regulator are fine; what died is the USB power path (the VBUS diode), so only
~3.4 V reaches 5Vin from USB. Powered from the buck on the 5Vin pin it boots
clean, joins WiFi (192.168.1.227) and OTA is up. Serial/esptool over USB still
work for data (bootloader flashes fine at the low voltage).
Third S3 (MAC 44:BD:8D:68:04:74, port `/dev/cu.usbmodem5CF70340961`) flashed
with `outside` 2026-10-20, then suffered the same USB-path damage in a strip
test; regulator fine, 3.3 V on buck power. It still holds `outside_pixeltest`
(no WiFi, no OTA) and did not answer esptool afterwards — park it.
Rules: strips get power only from 5Vin (never 3V3); beep-check +V to GND on the
strip AND on the board pins before power; nothing at 12 V ever touches the
board. The installed 5 V chain's first LED or its data wire is dead (a healthy
board with verified data still shows 5 stuck LEDs). Owner has a spare 12 V
WS2815 strip (GND/BI/DI/+12V pads) as the candidate replacement, fed from the
12 V side. 2026-10-21: DECIDED. Outside unit rebuilt on board 2 (buck power):
nine pads + spare amp play correctly. The old 5 V chain is dead even on the
buck rail (stuck LEDs, no data response). A 3-LED WS2815 test piece fed from
the 12 V side, DI straight from GPIO13 (no level shifter), BI to GND, flashes
correctly on pads 7-9. Build plan: three WS2815 pieces, +12V/GND from the 12 V
side before the buck, DI<-GPIO13 and BI->GND on the first, DO->DI and BO->BI
between pieces; no SS34, no 1000 uF. ABANDONED 2026-10-21: during the full
9-LED WS2815 build, 12 V most likely reached GPIO13 (the +12V pad sits next to
DI on that strip) and board 2 died too. Decision: back to a 5 V WS2812B 30/m
strip (fits the channels as designed), SS34 on its +5V lead, and the 330R in
series at the first DIN (it was missing from the rebuild). Firmware mapping is
back to 9 LEDs in reading order (`kPixelsOf` in src/main.cpp). On order: 2x
S3 N16R8, 5 V strip, 2x MAX98357A, diodes. Board 2's 3V3-on-buck reading was
not taken; if it reads 3.3 later, only GPIO13 may be dead (move pixel data to
another pin).
2026-10-30: fourth S3 (MAC 7C:E8:B1:AA:D6:D8, port `/dev/cu.usbmodem5C831108341`)
flashed with `outside` bare over USB: boots clean, WiFi 192.168.1.245, OTA up,
all pads idle high. A fifth S3 is sealed as the spare. New 5 V strip arrived.
Quirk of this board revision: the 5Vin pin does NOT source USB power (an
IN-OUT solder jumper by that corner is open from the factory; it read 2.2 V
floating), so on the bench the strip/amp take 5 V from the buck with a common
GND wire to the board while the board stays on USB. The old MP1584 was found
dead (dragged 12 V to 2.2 V by itself, a casualty of the 12 V strip incident);
a fresh MP1584 is in, set from 5.3 V toward 5.0 V. New 9-LED WS2812B piece,
owner-soldered lead (red +5V, green DIn, white GND): full chain verified on
board 4 with the real firmware (pad flash + chime all-nine flash).
2026-10-31: full outside unit on buck power only (no USB): board 4, amp, all
nine pads and the 9-LED strip verified by hand (sound + LED flash per pad).
Buck still at 5.3 V; strip data works without the SS34 at that voltage.
Strip cut into three 3-LED pieces and chained; the middle piece ended up
data-in on the right, so row 2 is {5},{4},{3} in `kPixelsOf` (OTA'd, works).
2026-10-31 install: IRM-10-12 (pin type, wires soldered to pins) replaced the
transformer in the chime box; 12 V DC on the door pair. Outside unit ran on the
pair, then died while the shell was being closed (board got hot). Board 4 now
reads 1.1 V on 3V3 with only 5 V in -> DEAD. Buck still 5.3 V. Cause is a
mechanical short when the faceplate pressed parts onto the header pins (speaker
terminals / 12 V side / bare button tabs suspected). Board 5 (spare) must not
go in until the pin rows and every bare metal part are insulated.
Board 5 (last spare, MAC B8:1F:3F:C3:D0:30, port `/dev/cu.usbmodem5C834152081`)
flashed bare with `outside` 2026-10-31: boots, WiFi 192.168.1.29, OTA up.
Board 5 then verified on the bench piece by piece (strip, amp, nine pads, then
buck power with USB out), every bare metal part (board pin rows, amp pins,
buck, speaker terminals, button tabs, strip cut ends) taped, shell closed:
WORKS closed, on buck power. Next: connect at the door (measure pair polarity
first: no reverse-polarity protection), then inside unit (needs an amp).
2026-10-31 INSTALLED: outside unit mounted at the door on the 12 V pair from
the IRM-10-12, playing and lighting (video IMG_7119). Remaining: inside unit
(needs an amp), volume / quiet hours / idle LED tuning.
Inside unit 2026-10-31: WROOM (port `/dev/cu.usbserial-0001`, ESP-NOW MAC
00:70:07:82:C1:1C) flashed with `inside`, WiFi 192.168.1.180, OTA up, new amp
+ 3" speaker: BOOT chime plays and it follows the installed door unit (link
UP through the door, sync error ~17 samples, 0 late). Knob off at that point
(DJ_VOLUME_POT=0; on since 2026-11-05). Then moved to its own MP1584 + SS34 off the chime-box 12 V: BOOT chime plays on wall power, USB out.
USB exception: boards 2 and 3 may have USB (data) plugged while buck-powered,
because their USB power path is already open; never do this on a healthy board.
Inside: 230V -> IRM-10-12 -> 12V -> (pair to door) and (MP1584 5.0V -> SS34
-> ~4.6V rail -> WROOM 5V, MAX98357A VIN).

The SS34 on the strip's +5V lead drops pixel VDD so 3.3V data clears the
0.7*VDD threshold (without it the bench chain showed all-red garbage).

## Amp config

- SD pin unconnected on both. Firmware writes identical L and R samples.
- GAIN: outside tied to VIN (6 dB, quiet feedback); inside floating (9 dB).
- Bridge-tied output: no speaker lead to ground, ever.

## After a power cut (2026-11-07)

The router comes back minutes after the units. The outside unit gives up WiFi
20 s after boot and used to stay off it until the next reboot (no web page, no
OTA, no clock, no notification; the doorbell itself kept working over ESP-NOW).
Since 2026-11-07 it retries the router every 2 min while the pad is idle
(`kWifiRetryMs` in src/radio.cpp). The inside unit always kept retrying.

## Remote presses (outside unit web listener, 2026-10-31)

The outside unit serves plain HTTP on port 80 inside the home network:
`GET /` is a phone-sized page with the nine pads + chime (src/web_page.h,
open it on the iPhone and Add to Home Screen); `GET /pad/1`..`/pad/9` presses
a pad (same path as a finger: lights, door speaker, inside chime), `GET /chime`
plays the plain chime, `GET /quiet` / `GET /quiet/set?...` quiet hours, `GET /notify`
/ `/notify/set` / `/notify/test` phone notification (see Features), `GET /help`
lists them. The page has Quiet hours and Phone notification cards at the bottom.
`curl http://djdoorbell-outside.local/pad/5` from a laptop or Pi; on an
iPhone, a Shortcut with "Get contents of URL" (add to home screen as a button).
No auth: LAN only.

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
board_build.partitions = min_spiffs.csv
build_flags = -DUNIT_INSIDE
```

WROOM-32D uses min_spiffs.csv: two 1.9 MB app slots, so the installed unit
can be updated over WiFi (OTA writes the inactive slot, then switches). The
default table's 1.25 MB slots are too small for firmware + samples;
huge_app.csv (one 3 MB slot) has room but cannot do OTA. Budget: firmware +
samples must stay under 1.9 MB (885 KB today). Changing the partition table
always needs one USB flash.

Bench phases 1-5 run on USB power only. The full bring-up sequence with pass
criteria lives in the build guide.

---

## Features

Confirmed, all zero-cost:
- **Performance replay.** Record events (pad index + grid position), not audio.
  A whole performance is under 100 bytes. Inside unit replays from the same
  samples. Gives every visitor a signature.
- **Quiet hours.** DONE 2026-11-05 (src/quiet.cpp). The outside unit gets
  local time by NTP (Israel TZ rule hardcoded), keeps settings in NVS and
  applies them itself; the inside level rides in the `volume` byte of every
  ESP-NOW packet and scales the knob. Settings on the pad web page
  (`GET /quiet` JSON, `GET /quiet/set?on=1&from=1320&to=420&out=25&in=50`,
  minutes after midnight, percent of normal, squared like the knob). Default
  off, 22:00-07:00, door 25 %, inside 50 %. Unknown clock = not quiet.
- **Phone notification.** DONE 2026-11-05 (src/notify.cpp, outside unit): ntfy
  push (free app, no account). One message when a performance starts (first
  press of a bed session) and one per plain chime, "(quiet hours)" appended
  when active. HTTPS POST on its own task on core 0, so the pads never wait.
  Settings in NVS, on the pad page (`GET /notify`, `/notify/set?on=1&topic=..
  &server=https://ntfy.sh`, `/notify/test`). Topic is the only secret.
- **Plain-chime escape hatch.** Long-press the centre pad plays a normal
  2-second chime. Delivery drivers and plumbers want one button and to leave.

---

## Build order

Status 2026-10-03: link test (3) PASSED; two-board firmware working on the
bench (4, knob still off). Previously: 1 and 2 done (bench firmware verified on the S3 with 5 pads,
quantiser exact on hardware). Enclosure modelled and mostly printed (5).
**Next: 6, solder the outside unit (LED strips first).** Full step-by-step plan for build day, and the
facts a new session needs: `docs/HANDOFF.md` — read it first.

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
