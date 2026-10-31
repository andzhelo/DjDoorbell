// DJ Doorbell — inside unit (chime) enclosure parameters. All dimensions in mm.
//
// The old chime box turned out too small for the 3" speaker + WROOM + buck +
// amp, so the inside unit gets its own box in the style of the outside one:
// a flat plate with the speaker grille (printed face-down) on an open shell
// (printed open side up), M3 heat-set inserts in the corners.
//
// Values marked MEASURE are placeholders until the real part is measured.

$fn = 96;

// ---- speaker (3" 4 ohm 5 W full-range) --------------------------------------
spk_d           = 78;     // MEASURE: frame outer diameter (round frame assumed)
spk_depth       = 36;     // MEASURE: frame front to magnet back
spk_cone_d      = 66;     // MEASURE: cone / surround opening = grille diameter
spk_hole_pcd    = 0;      // MEASURE: mounting-hole circle diameter, 0 = no screws
spk_hole_d      = 3.4;    // M3 clearance through the plate
spk_hole_angle  = 45;     // first mounting hole, degrees from +x
spk_ring_h      = 3;      // retaining ring on the back of the plate
spk_ring_wall   = 1.5;
grille_hole_d   = 2.2;
grille_pitch    = 3.4;

// ---- boards, lying flat on the shell floor on foam tape ----------------------
// Footprints only (no mounting holes on any of them); used for the phantoms
// in the assembly check and to size the bay.
wroom_l         = 55;     // MEASURE: 38-pin DevKit ~55 x 28, 30-pin ~51 x 28
wroom_w         = 28;
wroom_h         = 13;     // pins + USB socket, from the floor
buck_l          = 22;     // MP1584EN module
buck_w          = 17;
amp_l           = 18;     // MAX98357A breakout
amp_w           = 16;
board_gap       = 4;      // free space between boards

// ---- mains compartment: IRM-10-12 + the two lever connectors ---------------
// Sits over the wall box (the 230 V wires come out there), separated from the
// low-voltage side by a full-height partition with a notch for the 12 V pair.
irm_l           = 45.7;   // IRM-10-12 body, lying flat
irm_w           = 25.4;
irm_h           = 21.5;
wago_l          = 19;     // 221-413 lever connector
wago_w          = 13;
wago_h          = 8.5;
mains_bay_h     = irm_w + 2 * 6 + 2;   // 39.4
partition_t     = 1.6;
notch_w         = 8;      // 12 V pair passes the partition here

// ---- volume knob (10k pot on GPIO34; DJ_VOLUME_POT is still 0 in firmware) --
pot_hole_d      = 0;      // 0 = no knob hole; 7.2 for a 6 mm-shaft pot bushing
pot_xy          = [0, 0]; // set below once the plate is sized

// ---- plate --------------------------------------------------------------------
panel_t         = 2.5;
corner_r        = 6;
margin          = 6;      // ring / board to the inner wall
bay_h           = wroom_w + 2 * margin + 4;   // electronics bay under the speaker
spk_ring_od     = spk_d + 0.4 + 2 * spk_ring_wall;
plate_w         = max(spk_ring_od + 2 * margin + 6,
                      wroom_l + board_gap + max(buck_l, amp_l) + 2 * margin + 6);
plate_h         = bay_h + spk_ring_od + 2 * margin + mains_bay_h;

// ---- shell --------------------------------------------------------------------
wall_t          = 2.4;
floor_t         = 2.4;
inner_depth     = max(spk_depth + spk_ring_h + 2, wroom_h + 4);
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
spk_cx          = plate_w / 2;
spk_cy          = bay_h + margin + spk_ring_od / 2;
mains_y0        = plate_h - mains_bay_h;          // partition sits here
mains_cy        = mains_y0 + mains_bay_h / 2;     // wall box centre
bay_cy          = bay_h / 2 + 2;
shell_h         = floor_t + inner_depth;

// Board positions (bottom-left corner of each footprint, plate coordinates).
wroom_xy        = [wall_t + margin, bay_cy - wroom_w / 2];
buck_xy         = [wroom_xy[0] + wroom_l + board_gap, bay_cy + 1];
amp_xy          = [wroom_xy[0] + wroom_l + board_gap, bay_cy - 1 - amp_w];

corner_xy = [[screw_inset, screw_inset],
             [plate_w - screw_inset, screw_inset],
             [screw_inset, plate_h - screw_inset],
             [plate_w - screw_inset, plate_h - screw_inset]];

module rounded_rect(w, h, r) {
  hull() for (x = [r, w - r], y = [r, h - r]) translate([x, y]) circle(r = r);
}

assert(buck_xy[0] + buck_l < plate_w - wall_t - margin, "buck hits the right wall");
assert(spk_cy + spk_ring_od / 2 < mains_y0 - 0.5, "speaker ring hits the mains partition");
assert(irm_l + wago_l + 3 * 4 < plate_w - 2 * wall_t, "IRM + connector do not fit across the mains bay");
assert(spk_cy - spk_ring_od / 2 > bay_h, "speaker ring overlaps the board bay");
