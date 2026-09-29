// DJ Doorbell — outside unit enclosure parameters. All dimensions in mm.
//
// Every value marked MEASURE is a placeholder until the real part is in hand
// with calipers. Change it here only; the coupon, faceplate and shell all
// include this file. Print the coupon before the faceplate.

$fn = 96;

// ---- arcade buttons (24 mm Sanwa-clone snap-in) ----------------------------
btn_hole_d      = 24.0;   // coupon result: 24.0/2.5 fit best
btn_flange_d    = 27.5;   // MEASURE: bezel outer diameter (sets clearances)
// Ruler photo, +-1 mm, all measured from the bezel underside (panel face):
btn_body_h      = 16;     // bottom of the clear body
btn_switch_h    = 20;     // bottom of the moulded switch block
btn_depth       = 26;     // tip of the terminals; solder joints add ~1
btn_pitch       = 30;     // spec: 30 mm centre-to-centre
btn_body_d      = 23.8;   // clear body below the bezel (snug in the 24.0 hole)
panel_t         = 2.5;    // coupon result: 24.0/2.5 fit best
coupon_holes    = [24.0, 24.2, 24.4, 24.6];   // one tile per diameter...
coupon_thick    = [2.5, 3.0];                 // ...at each panel thickness

// ---- faceplate -------------------------------------------------------------
// 114, not 100: a 3-LED piece of 30/m strip is 100 mm long and has to fit
// inside the rib with room for the row-to-row wires at its ends.
plate_w         = 114;
pad_h           = 100;    // pad region height (3 x 30 pitch + margins)
bay_h           = 45;     // electronics + speaker bay below the pad
plate_h         = pad_h + bay_h;   // 145
corner_r        = 6;

// ---- speaker (23/24 mm 4 ohm 2 W) -----------------------------------------
spk_d           = 23.5;   // MEASURE: speaker outer diameter (frame)
spk_ring_h      = 3;      // retaining ring on the back of the plate
grille_hole_d   = 2.0;
grille_pitch    = 3.2;
grille_d        = 20;     // grille pattern diameter (cone opening)

// ---- shell -----------------------------------------------------------------
wall_t          = 2.4;    // 6 perimeters at 0.4 mm
floor_t         = 2.4;
// Room behind the terminals for soldered wires to bend over (~8 mm).
// Measured from the shell floor to the rim (= faceplate back).
inner_depth     = btn_depth + 8 - panel_t;
screw_inset     = 6;      // M3 corner screws, from the plate edges
boss_d          = 7;
insert_hole_d   = 4.0;    // M3 heat-set insert (4.0 x 5.7 typical)
insert_hole_h   = 6.5;
screw_clear_d   = 3.4;    // M3 clearance
screw_head_d    = 6.2;    // countersunk head
rib_h           = 3;      // locating rib on the plate back, sits inside the shell
fit_clear       = 0.25;   // rib-to-wall clearance

// ---- wall interface --------------------------------------------------------
wire_hole_d     = 12;     // rear wire entry, centred on the wall box
// European 60 mm box standard; horizontal slots take 52-68 mm spacing.
wallbox_slot_sp = 60;     // MEASURE if the box is non-standard
wallbox_slot_w  = 4.5;
wallbox_slot_l  = 12.5;

// ---- board standoffs in the bay --------------------------------------------
// The DevKitC-1 has no mounting holes, so no standoffs: boards go on the
// bay floor with foam tape. Add [x, y] offsets from the bay centre here for
// any board that does have holes.
board_holes     = [];
standoff_d      = 6;
standoff_hole_d = 2.2;    // M2.5 self-tapping
standoff_h      = 5;

// ---- LED strips (side lighting) ---------------------------------------------
// The switch block covers the bottom of the button, so each row is lit from
// the side: a 3-LED piece of strip stands on edge in a channel on the plate
// back, just below its row, LEDs facing up into the clear bodies.
strip_len       = 100;    // 3 LEDs at 33.33 mm (30/m), cut at the pads
strip_w         = 10;     // strip width = how far it stands off the plate
strip_t         = 2.0;    // FPC + 5050 LED + tape
strip_gap       = 0.8;    // LED face to button body
fin_t           = 1.2;    // backing wall the strip's adhesive sticks to
fin_len         = 96;
lip             = 1.0;    // retaining lip at the fin's free edge
lip_t           = 1.2;
// Strip back face, relative to its row centre (negative = below the row).
function strip_back_y(row_y) = row_y - btn_body_d / 2 - strip_gap - strip_t;

// ---- derived ---------------------------------------------------------------
pad_cx          = plate_w / 2;
pad_cy          = bay_h + pad_h / 2;      // pad grid centre
bay_cx          = plate_w / 2;
bay_cy          = bay_h / 2;
shell_h         = floor_t + inner_depth;

// Pad centres in reading order (top-left first), pad index 0-8.
function pad_xy(i) = [pad_cx + ((i % 3) - 1) * btn_pitch,
                      pad_cy + (1 - floor(i / 3)) * btn_pitch];

// Corner screw positions.
corner_xy = [[screw_inset, screw_inset],
             [plate_w - screw_inset, screw_inset],
             [screw_inset, plate_h - screw_inset],
             [plate_w - screw_inset, plate_h - screw_inset]];

module rounded_rect(w, h, r) {
  hull() for (x = [r, w - r], y = [r, h - r]) translate([x, y]) circle(r = r);
}
