# Enclosure — outside unit

Parametric OpenSCAD. `params.scad` holds every dimension; the parts include it.
Values marked `MEASURE` are placeholders until the real parts are measured.

    cad/render.sh              # all parts -> stl/ and preview/
    cad/render.sh faceplate    # one part

| Part | What | Print |
|---|---|---|
| `coupon.scad` | 3 tiles, one per candidate button hole (24.0/24.2/24.4) at the panel thickness | hole-up, same settings as the plate |
| `faceplate.scad` | 100 x 145 plate: 3x3 buttons on 30 mm pitch, speaker grille + retaining ring in the 45 mm bay, locating rib, 4 countersunk M3 | face-down on a smooth bed |
| `shell.scad` | open box behind the plate: M3 heat-set bosses, rear wire entry + wall-box slots under the pad, board standoffs in the bay | open side up, no supports |

Order: print the coupon, snap a button into each tile, set `btn_hole_d` and
`panel_t`. Then the plate, then the shell. PETG for both.

The inside unit reuses the existing chime enclosure (see CLAUDE.md); only a
carrier tray for the boards may be printed, once that box is open and measured.

## Measure before the faceplate

- Button: hole diameter (from the coupon), flange diameter, depth behind the
  panel with the microswitch, panel thickness range the tabs accept.
- Speaker frame diameter.
- Nisko wall box: screw-lug spacing, outer size, depth.
- DevKitC-1 mounting holes, if any; else the standoffs become rail clips.
- A cut WS2812 segment must fit the vacated lamp cavity — check first.
