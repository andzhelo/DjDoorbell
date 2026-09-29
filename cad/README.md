# Enclosure — outside unit

Parametric OpenSCAD. `params.scad` holds every dimension; the parts include it.
Values marked `MEASURE` are placeholders until the real parts are measured.

    cad/render.sh              # all parts -> stl/ and preview/
    cad/render.sh faceplate    # one part

| Part | What | Print |
|---|---|---|
| `coupon.scad` | 8 tiles: hole 24.0/24.2/24.4/24.6 x panel 2.5/3.0 mm, labelled hole/thickness | hole-up, same settings as the plate |
| `faceplate.scad` | 100 x 145 x 2.5 plate: 3x3 Ø24.0 holes on 30 mm pitch, speaker grille + retaining ring in the 45 mm bay, locating rib, 4 countersunk M3 | face-down on a smooth bed |
| `shell.scad` | open box, 33.9 mm deep: M3 heat-set bosses, rear wire entry + horizontal wall-box slots (52-68 mm) under the pad, ledges for the LED carrier | open side up, no supports |
| `assembly.scad` | plate on shell, for viewing; `-D interference=true` renders overlaps (must be empty) | not printed |

Coupon result: `24.0/2.5` fit best, so `btn_hole_d = 24.0`, `panel_t = 2.5`.
Plate and shell are printable; PETG for both. Hardware: 4x M3 heat-set
inserts (4.0 mm hole), 4x M3 x 8 countersunk screws (x 10 would bottom out in the 6.5 mm insert hole).

The buttons are OBSC-24 clones (XW-OBSC): frosted plunger, clear body, microswitch
moulded into the base (not removable), 2.8 mm tabs, no lamp holder. The WS2812 sits
beside the switch block under the clear body, on an LED carrier layer in the
shell (to be modelled once the switch offset and depth are measured).

The inside unit reuses the existing chime enclosure (see CLAUDE.md); only a
carrier tray for the boards may be printed, once that box is open and measured.

## Measure before the faceplate

- Switch block footprint and offset from the button centre (for the LED carrier).
- Speaker frame diameter (the retaining ring assumes 23.5 mm).
- Nisko wall box: screw-lug spacing, outer size, depth.
- DevKitC-1 mounting holes, if any; else the standoffs become rail clips.
- A cut WS2812 segment must fit the vacated lamp cavity — check first.
