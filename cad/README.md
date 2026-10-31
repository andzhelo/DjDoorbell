# Enclosure — outside unit

Parametric OpenSCAD. `params.scad` holds every dimension; the parts include it.
Values marked `MEASURE` are placeholders until the real parts are measured.

    cad/render.sh              # all parts -> stl/ and preview/
    cad/render.sh faceplate    # one part

| Part | What | Print |
|---|---|---|
| `coupon.scad` | 8 tiles: hole 24.0/24.2/24.4/24.6 x panel 2.5/3.0 mm, labelled hole/thickness | hole-up, same settings as the plate |
| `faceplate.scad` | 114 x 145 x 2.5 plate: 3x3 Ø24.0 holes on 30 mm pitch, 3 LED strip channels on the back, speaker grille + retaining ring in the 45 mm bay, locating rib, 4 countersunk M3 | face-down on a smooth bed |
| `shell.scad` | open box, 33.9 mm deep: M3 heat-set bosses, rear wire entry + horizontal wall-box slots (52-68 mm) under the pad | open side up, no supports |
| `assembly.scad` | plate on shell with button and strip phantoms; `-D 'check="shell"'` / `"buttons"` / `"strips"` render overlaps (all must be empty) | not printed |

Coupon result: `24.0/2.5` fit best, so `btn_hole_d = 24.0`, `panel_t = 2.5`.
Plate and shell are printable; PETG for both. Hardware: 4x M3 heat-set
inserts (4.0 mm hole), 4x M3 x 8 countersunk screws (x 10 would bottom out in the 6.5 mm insert hole).

The buttons are OBSC-24 clones (XW-OBSC): frosted plunger, clear body, microswitch
moulded into the base (not removable), 2.8 mm tabs, no lamp holder. The switch
block covers almost the whole base, so the buttons are lit from the side
(hand-tested: the cap glows). Each row gets one uncut 3-LED piece of the 30/m
strip (100 mm), standing on edge in a channel on the plate back just below the
row, LEDs facing up into the clear bodies. Slide it in from the end of the
channel, adhesive side against the fin. The plate is 114 wide, not 100, so the
100 mm strip fits inside the rib with ~2 mm at each end for the row-to-row wires.
Chain order: row 1 left to right, then row 2, then row 3 — pixel index = pad index.

## Inside unit — `inside_*.scad`

The old chime box is too small for the 3" speaker + WROOM + buck + amp, so the
inside unit gets its own box in the same style (2026-10-31). `inside_params.scad`
holds its dimensions; the three `MEASURE` values that size everything are the
speaker frame diameter, its depth and its cone opening (plus the mounting-hole
circle if the frame has holes).

| Part | What | Print |
|---|---|---|
| `inside_plate.scad` | plate with the speaker grille, retaining ring on the back, optional volume-knob hole, 4 countersunk M3 | face-down |
| `inside_shell.scad` | open box: M3 bosses, 12 V wire entry + wall-box slots behind the speaker, a low fence keeping the boards out from under the magnet | open side up |
| `inside_assembly.scad` | plate on shell with speaker + board phantoms; `-D 'check="fit"'` must be empty | not printed |

Layout: speaker in the upper part hanging from the plate ring, WROOM + buck +
amp flat on the shell floor in the bay below it (foam tape), every pin row
taped over before closing (that is what killed outside board 4).
The IRM-10-12 stays in the old chime box; only the 12 V pair comes over.

## Status

Coupon done; shell printed. Faceplate needs a reprint for the 40 mm speaker.
Nisko wall-box spacing is still assumed (60 mm, slots take 52-68).
