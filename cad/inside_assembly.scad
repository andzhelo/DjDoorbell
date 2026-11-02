// Inside plate on its shell with speaker and board phantoms. Not for printing.
//   check = "none"    show the assembly
//   check = "shell"   shell vs plate overlap                          (must be empty)
//   check = "fit"     speaker + boards + mains parts vs plate + shell (must be empty)
//   check = "open"    shell with the parts in place, plate removed (layout picture)
include <inside_params.scad>
use <inside_plate.scad>
use <inside_shell.scad>

check = "none";

module place() { translate([0, 0, shell_h + panel_t]) mirror([0, 0, 1]) children(); }

// speaker: square frame on the standoff bosses, basket + magnet behind it;
// the pot body hangs from the plate beside the boards
module speaker() {
  translate([spk_cx, spk_cy, panel_t + spk_boss_h + 0.01]) {
    translate([-spk_d / 2, -spk_d / 2, 0]) cube([spk_d, spk_d, 2]);
    cylinder(d = spk_cone_d, h = spk_depth * 0.5);
    cylinder(d = spk_magnet_d, h = spk_depth - spk_boss_h);
  }
  if (pot_hole_d > 0)
    translate([pot_xy_set[0], pot_xy_set[1], panel_t + 0.01])
      linear_extrude(pot_body_h) square(pot_body_sq, center = true);
}

// boards on the shell floor (shell coordinates)
module boards() {
  translate([wroom_xy[0], wroom_xy[1], floor_t + 0.01]) cube([wroom_w, wroom_l, wroom_h]);   // lengthwise
  translate([buck_xy[0],  buck_xy[1],  floor_t + 0.01]) cube([buck_l,  buck_w,  5]);
  translate([amp_xy[0],   amp_xy[1],   floor_t + 0.01]) cube([amp_l,   amp_w,   5]);
  // mains compartment: IRM lying flat, the two lever connectors stacked beside it
  translate([irm_xy[0], irm_xy[1], floor_t + 0.01]) cube([irm_l, irm_w, irm_h]);
  translate([wago_x, irm_xy[1] + (irm_w - wago_w) / 2, floor_t + 0.01]) cube([wago_l, wago_w, 2 * wago_h]);
}

if (check == "shell")
  intersection() { inside_shell(); translate([0, 0, 0.01]) place() inside_plate(); }
else if (check == "fit")
  intersection() {
    union() { place() speaker(); boards(); }
    union() { inside_shell(); translate([0, 0, 0.01]) place() inside_plate(); }
  }
else if (check == "open") {
  color("gold") inside_shell();
  color("lightgreen") boards();
  place() color("dimgray") speaker();
}
else {
  color("gold") inside_shell();
  color("lightgreen") boards();
  place() { color("white", 0.85) inside_plate(); color("dimgray") speaker(); }
}
