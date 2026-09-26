// Shell: open box behind the faceplate. Corner bosses take M3 heat-set
// inserts; the floor has the wire entry and slots for the wall-box screws
// under the pad, and board standoffs in the bay.
// Printed open side up, no supports.
include <params.scad>

module shell() {
  difference() {
    union() {
      difference() {
        linear_extrude(shell_h) rounded_rect(plate_w, plate_h, corner_r);
        translate([0, 0, floor_t])
          linear_extrude(shell_h)
            offset(delta = -wall_t) rounded_rect(plate_w, plate_h, corner_r);
      }
      // corner bosses
      for (p = corner_xy) translate([p[0], p[1], 0]) cylinder(d = boss_d, h = shell_h);
      // board standoffs
      for (h = board_holes)
        translate([bay_cx + h[0], bay_cy + h[1], floor_t - 0.01])
          cylinder(d = standoff_d, h = standoff_h);
    }

    // heat-set insert holes
    for (p = corner_xy)
      translate([p[0], p[1], shell_h - insert_hole_h]) cylinder(d = insert_hole_d, h = insert_hole_h + 1);

    // standoff screw holes
    for (h = board_holes)
      translate([bay_cx + h[0], bay_cy + h[1], floor_t - 1])
        cylinder(d = standoff_hole_d, h = standoff_h + 2, $fn = 24);

    // wire entry, centred on the wall box behind the pad
    translate([pad_cx, pad_cy, -1]) cylinder(d = wire_hole_d, h = floor_t + 2);

    // wall-box mounting slots
    for (s = [-1, 1])
      translate([pad_cx + s * wallbox_slot_sp / 2, pad_cy, -1])
        hull() for (y = [-1, 1])
          translate([0, y * (wallbox_slot_l - wallbox_slot_w) / 2, 0])
            cylinder(d = wallbox_slot_w, h = floor_t + 2, $fn = 32);
  }
}

shell();
