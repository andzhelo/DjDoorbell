// Inside unit shell: open box behind the plate. Corner bosses take M3
// heat-set inserts; the floor has the 12 V wire entry and wall-box slots
// behind the speaker, and a low fence around the board bay so the boards
// cannot slide under the speaker magnet. Printed open side up, no supports.
include <inside_params.scad>

fence_h = 4;
fence_t = 1.2;

module inside_shell() {
  difference() {
    union() {
      difference() {
        linear_extrude(shell_h) rounded_rect(plate_w, plate_h, corner_r);
        translate([0, 0, floor_t])
          linear_extrude(shell_h)
            offset(delta = -wall_t) rounded_rect(plate_w, plate_h, corner_r);
      }
      for (p = corner_xy) translate([p[0], p[1], 0]) cylinder(d = boss_d, h = shell_h);
      // fence between the board bay and the speaker
      translate([wall_t, bay_h, floor_t - 0.01]) cube([plate_w - 2 * wall_t, fence_t, fence_h]);
    }

    for (p = corner_xy)
      translate([p[0], p[1], shell_h - insert_hole_h]) cylinder(d = insert_hole_d, h = insert_hole_h + 1);

    // wire entry and wall-box slots, centred behind the speaker
    translate([spk_cx, spk_cy, -1]) cylinder(d = wire_hole_d, h = floor_t + 2);
    for (s = [-1, 1])
      translate([spk_cx + s * wallbox_slot_sp / 2, spk_cy, -1])
        hull() for (x = [-1, 1])
          translate([x * (wallbox_slot_l - wallbox_slot_w) / 2, 0, 0])
            cylinder(d = wallbox_slot_w, h = floor_t + 2, $fn = 32);
  }
}

inside_shell();
