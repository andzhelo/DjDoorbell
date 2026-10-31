// Inside plate on its shell with speaker and board phantoms. Not for printing.
//   check = "none"    show the assembly
//   check = "fit"     speaker + boards vs plate + shell overlap (must be empty)
include <inside_params.scad>
use <inside_plate.scad>
use <inside_shell.scad>

check = "none";

module place() { translate([0, 0, shell_h + panel_t]) mirror([0, 0, 1]) children(); }

// speaker: frame disc + magnet, hanging from the plate back
module speaker() {
  translate([spk_cx, spk_cy, panel_t + 0.01]) {
    cylinder(d = spk_d, h = 4);
    cylinder(d = spk_d * 0.55, h = spk_depth);
  }
}

// boards on the shell floor (shell coordinates)
module boards() {
  translate([wroom_xy[0], wroom_xy[1], floor_t + 0.01]) cube([wroom_l, wroom_w, wroom_h]);
  translate([buck_xy[0],  buck_xy[1],  floor_t + 0.01]) cube([buck_l,  buck_w,  5]);
  translate([amp_xy[0],   amp_xy[1],   floor_t + 0.01]) cube([amp_l,   amp_w,   5]);
}

if (check == "fit")
  intersection() {
    union() { place() speaker(); boards(); }
    union() { inside_shell(); translate([0, 0, 0.01]) place() inside_plate(); }
  }
else {
  color("gold") inside_shell();
  color("lightgreen") boards();
  place() { color("white", 0.85) inside_plate(); color("dimgray") speaker(); }
}
