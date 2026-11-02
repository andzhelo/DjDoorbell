// DJ Doorbell — inside unit (chime) enclosure parameters. All dimensions in mm.
//
// The old chime box turned out too small for the 3" speaker + WROOM + buck +
// amp, so the inside unit gets its own box in the style of the outside one:
// a flat plate with the speaker grille (printed face-down) on an open shell
// (printed open side up), M3 heat-set inserts in the corners.
//
// Layout (2026-11-02, owner wants no side over 130 mm): speaker at the bottom
// left, a column of boards to its right, and the mains compartment as a strip
// across the top left over the wall box. 130 x 126 mm.
//
// Values marked MEASURE are placeholders until the real part is measured.

$fn = 96;

// ---- speaker (3" 4 ohm 5 W full-range, square frame) -------------------------
// From the listing (2026-11-01): 78 x 78 mm square frame, 4 mounting holes on
// an 84 mm diagonal, 45 mm magnet. The frame hangs on four standoff bosses on
// the back of the plate (M3 heat-set inserts), screws from the speaker side.
spk_d           = 78;     // frame outer size (square)
spk_depth       = 36;     // MEASURE: frame front to magnet back (listing does not say)
spk_cone_d      = 68;     // grille diameter = the frame's round opening
spk_magnet_d    = 45;
spk_hole_diag   = 84;     // hole-to-hole distance across the diagonal
spk_hole_off    = spk_hole_diag / 2 / sqrt(2);   // 29.7: hole offset from centre on each axis
spk_boss_d      = 7;
spk_boss_h      = 6;      // standoff: the surround sits in front of the frame, keep it off the plate
spk_insert_h    = 5;      // M3 x 5 heat-set insert (plate + boss = 8.5 mm total)
grille_hole_d   = 2.2;
grille_pitch    = 3.4;

// ---- boards, lying flat on the shell floor on foam tape ----------------------
// Footprints only (no mounting holes on any of them); used for the phantoms
// in the assembly check and to size the column. The WROOM stands lengthwise.
wroom_l         = 55;     // MEASURE: 38-pin DevKit ~55 x 28, 30-pin ~51 x 28
wroom_w         = 28;
wroom_h         = 13;     // pins + USB socket, from the floor
buck_l          = 22;     // MP1584EN module
buck_w          = 17;
amp_l           = 18;     // MAX98357A breakout
amp_w           = 16;
board_gap       = 4;      // free space between boards

// ---- mains compartment: IRM-10-12 + the two lever connectors ---------------
// Strip across the top left, over the wall box (the 230 V wires come out
// there), walled off from the low-voltage side by a full-height partition
// with a notch for the 12 V pair.
irm_l           = 45.7;   // IRM-10-12 body, lying flat
irm_w           = 25.4;
irm_h           = 21.5;
wago_l          = 19;     // 221-413 lever connector; the two sit one on top of the other
wago_w          = 13;
wago_h          = 8.5;
partition_t     = 1.6;
mains_pad       = 4;      // IRM to the walls of its bay
mains_bay_h     = irm_w + 2 * mains_pad + partition_t;   // 35
notch_w         = 8;      // 12 V pair passes the partition here

// ---- volume knob (10k pot on GPIO34; DJ_VOLUME_POT is still 0 in firmware) --
pot_hole_d      = 7.2;    // M7 bushing of a 16 mm pot; 0 = no knob hole
pot_body_d      = 17;     // pot body behind the plate (phantom in the fit check)
pot_body_h      = 10;

// ---- plate --------------------------------------------------------------------
panel_t         = 2.5;
corner_r        = 6;
wall_t          = 2.4;
spk_margin      = 8;      // frame to the inner wall: clears the corner bosses
col_gap         = 4;      // speaker frame to the board column
col_w           = 30;     // board column (WROOM is 28 wide)
spk_env         = spk_d + 1;                 // square frame + clearance
col_x0          = wall_t + spk_margin + spk_env + col_gap;   // 93.4
plate_w         = 130;
plate_h         = spk_margin + spk_env + col_gap + mains_bay_h;   // 126

// ---- shell --------------------------------------------------------------------
floor_t         = 2.4;
inner_depth     = max(spk_depth + spk_boss_h + 2, wroom_h + 4);
screw_inset     = 6;
boss_d          = 7;
insert_hole_d   = 4.0;    // M3 heat-set insert
insert_hole_h   = 6.5;
screw_clear_d   = 3.4;
screw_head_d    = 6.2;
rib_h           = 3;
fit_clear       = 0.25;

// ---- wall interface -------------------------------------------------------------
wire_hole_d     = 12;     // 12 V pair from the chime box comes in here
wallbox_slot_sp = 60;     // MEASURE: 60 mm European box, slots take 52-68
wallbox_slot_w  = 4.5;
wallbox_slot_l  = 12.5;

// ---- derived ------------------------------------------------------------------
spk_cx          = wall_t + spk_margin + spk_env / 2;      // 49.9
spk_cy          = spk_margin + spk_env / 2;               // 47.5
mains_y0        = spk_margin + spk_env + col_gap;         // 91: horizontal partition
mains_x1        = col_x0 - col_gap / 2 - partition_t;     // 89.8: vertical partition
mains_cx        = (wall_t + mains_x1) / 2;                // wall box centre
mains_cy        = mains_y0 + partition_t + (mains_bay_h - partition_t) / 2;
shell_h         = floor_t + inner_depth;

// Board positions (bottom-left corner of each footprint, plate coordinates).
// Column, bottom to top: WROOM lengthwise, buck, amp; the pot hangs from the
// plate above the amp (33 mm of air between them).
wroom_xy        = [col_x0 + 1, screw_inset + boss_d / 2 + 1];          // clears the corner boss
buck_xy         = [col_x0 + 1, wroom_xy[1] + wroom_l + board_gap];
amp_xy          = [col_x0 + 1, buck_xy[1] + buck_w + board_gap];
pot_xy_set      = [col_x0 + col_w / 2 - 2, plate_h - wall_t - fit_clear - 2 - pot_body_d / 2 - 1];   // body clears the plate rib
irm_xy          = [screw_inset + boss_d / 2 + 1, mains_y0 + partition_t + mains_pad];   // clears the corner boss
wago_x          = irm_xy[0] + irm_l + mains_pad;

corner_xy = [[screw_inset, screw_inset],
             [plate_w - screw_inset, screw_inset],
             [screw_inset, plate_h - screw_inset],
             [plate_w - screw_inset, plate_h - screw_inset]];

module rounded_rect(w, h, r) {
  hull() for (x = [r, w - r], y = [r, h - r]) translate([x, y]) circle(r = r);
}

assert(plate_w <= 130 && plate_h <= 130, "a side is over 130 mm");
assert(wroom_xy[0] + wroom_w < plate_w - wall_t - 1, "WROOM hits the right wall");
assert(amp_xy[1] + amp_w < plate_h - screw_inset - boss_d / 2 - 1, "amp reaches the top corner boss");
assert(pot_xy_set[1] + pot_body_d / 2 < plate_h - wall_t - 1, "pot body hits the top wall");
assert(pot_xy_set[0] + pot_body_d / 2 < plate_w - screw_inset - boss_d / 2 - 0.5, "pot body hits the corner boss");
assert(spk_cy + spk_env / 2 < mains_y0 - 0.5, "speaker frame hits the mains partition");
assert(wago_x + wago_l + mains_pad < mains_x1, "IRM + connector do not fit across the mains bay");
assert(2 * wago_h + 2 < inner_depth, "the two stacked connectors are taller than the bay");
