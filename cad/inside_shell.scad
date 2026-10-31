// Inside unit shell: open box behind the plate. Corner bosses take M3
// heat-set inserts; the floor has the 12 V wire entry and wall-box slots
// over the wall box, a full-height partition walling off the mains
// compartment (IRM-10-12 + lever connectors) with a notch for the 12 V pair,
// and a low fence keeping the boards out from under the speaker magnet.
// Printed open side up, no supports.
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
      // mains partition, full height, notched at the left for the 12 V pair
      difference() {
        translate([wall_t, mains_y0, floor_t - 0.01])
          cube([plate_w - 2 * wall_t, partition_t, shell_h - floor_t - 0.5]);
        translate([wall_t + 8, mains_y0 - 1, shell_h - 12]) cube([notch_w, partition_t + 2, 13]);
        // clear the plate's locating rib at both ends
        for (x = [wall_t - 1, plate_w - wall_t - fit_clear - 2.3])
          translate([x, mains_y0 - 1, shell_h - rib_h - 0.3]) cube([fit_clear + 3.3, partition_t + 2, rib_h + 1]);
      }
    }

    for (p = corner_xy)
      translate([p[0], p[1], shell_h - insert_hole_h]) cylinder(d = insert_hole_d, h = insert_hole_h + 1);

    // wire entry and wall-box slots, centred in the mains compartment
    translate([spk_cx, mains_cy, -1]) cylinder(d = wire_hole_d, h = floor_t + 2);
    for (s = [-1, 1])
      translate([spk_cx + s * wallbox_slot_sp / 2, mains_cy, -1])
        hull() for (x = [-1, 1])
          translate([x * (wallbox_slot_l - wallbox_slot_w) / 2, 0, 0])
            cylinder(d = wallbox_slot_w, h = floor_t + 2, $fn = 32);
  }
}

inside_shell();
