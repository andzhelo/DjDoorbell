// DJ Doorbell — outside unit enclosure parameters. All dimensions in mm.
//
// Every value marked MEASURE is a placeholder until the real part is in hand
// with calipers. Change it here only; the coupon, faceplate and shell all
// include this file. Print the coupon before the faceplate.

$fn = 96;

// ---- arcade buttons (24 mm Sanwa-clone snap-in) ----------------------------
btn_hole_d      = 24.2;   // MEASURE: pick from the coupon (24.0 / 24.2 / 24.4)
btn_flange_d    = 27.5;   // MEASURE: bezel outer diameter (sets clearances)
btn_depth       = 32;     // MEASURE: depth behind the panel incl. microswitch
btn_pitch       = 30;     // spec: 30 mm centre-to-centre
panel_t         = 3.0;    // MEASURE: within the range the snap tabs accept (spec 2-3)
coupon_holes    = [24.0, 24.2, 24.4];   // one coupon per diameter

// ---- faceplate -------------------------------------------------------------
plate_w         = 100;    // spec: 100 mm pad width
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
inner_depth     = btn_depth + 3;   // clearance behind the deepest part
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
wallbox_slot_sp = 60;     // MEASURE: Nisko box screw-lug spacing
wallbox_slot_w  = 4.5;
wallbox_slot_l  = 10;     // vertical slots: 5 mm of adjustment

// ---- board standoffs in the bay --------------------------------------------
// MEASURE: ESP32-S3 DevKitC-1 mounting-hole positions relative to the bay
// centre. Many DevKitC boards have no holes at all; then use rail clips.
board_holes     = [[-30, -8], [30, -8], [-30, 8], [30, 8]];
standoff_d      = 6;
standoff_hole_d = 2.2;    // M2.5 self-tapping
standoff_h      = 5;

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
