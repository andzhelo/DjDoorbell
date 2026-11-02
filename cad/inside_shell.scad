// Inside unit shell: open box behind the plate. Corner bosses take M3
// heat-set inserts; the floor has the 12 V wire entry and wall-box slots
// over the wall box; an L-shaped full-height partition walls off the mains
// compartment (IRM-10-12 + lever connectors, top left) from the speaker and
// the board column, with a notch for the 12 V pair; a low fence keeps the
// boards from sliding under the speaker frame.
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

      // fence between the speaker and the board column
      translate([col_x0 - col_gap / 2 - fence_t / 2, wall_t, floor_t - 0.01])
        cube([fence_t, mains_y0 - wall_t, fence_h]);

      // mains partition: horizontal under the bay, vertical at its right end
      difference() {
        union() {
          translate([wall_t, mains_y0, floor_t - 0.01])
            cube([mains_x1 + partition_t - wall_t, partition_t, shell_h - floor_t - 0.5]);
          translate([mains_x1, mains_y0, floor_t - 0.01])
            cube([partition_t, plate_h - wall_t - mains_y0, shell_h - floor_t - 0.5]);
        }
        // notch for the 12 V pair, next to the board column, at the top of the partition
        translate([mains_x1 - 2 - notch_w, mains_y0 - 1, shell_h - 12]) cube([notch_w, partition_t + 2, 13]);
        // clear the plate's locating rib where the partition meets the walls
        translate([wall_t - 1, mains_y0 - 1, shell_h - rib_h - 0.3]) cube([fit_clear + 3.3, partition_t + 2, rib_h + 1]);
        translate([mains_x1 - 1, plate_h - wall_t - fit_clear - 2.3, shell_h - rib_h - 0.3]) cube([partition_t + 2, fit_clear + 3.3, rib_h + 1]);
      }
    }

    for (p = corner_xy)
      translate([p[0], p[1], shell_h - insert_hole_h]) cylinder(d = insert_hole_d, h = insert_hole_h + 1);

    // wire entry and wall-box slots, centred in the mains compartment
    translate([mains_cx, mains_cy, -1]) cylinder(d = wire_hole_d, h = floor_t + 2);
    for (s = [-1, 1])
      translate([mains_cx + s * wallbox_slot_sp / 2, mains_cy, -1])
        hull() for (x = [-1, 1])
          translate([x * (wallbox_slot_l - wallbox_slot_w) / 2, 0, 0])
            cylinder(d = wallbox_slot_w, h = floor_t + 2, $fn = 32);
  }
}

inside_shell();
