// Faceplate mounted on the shell, with button bodies and LED strips as
// phantoms, for checking fit. Not for printing.
//   check = "none"     show the assembly
//   check = "shell"    shell vs faceplate overlap      (must be empty)
//   check = "buttons"  buttons vs faceplate + strips   (must be empty)
//   check = "strips"   strips vs faceplate + shell     (must be empty)
include <params.scad>
use <faceplate.scad>
use <shell.scad>

check = "none";

// Everything below is in faceplate coordinates (front face at z = 0, back
// toward +z), then flipped onto the shell by place().
module place() { translate([0, 0, shell_h + panel_t]) mirror([0, 0, 1]) children(); }

module buttons() {
  for (i = [0 : 8]) translate([pad_xy(i)[0], pad_xy(i)[1], panel_t + 0.01])
    cylinder(d = btn_body_d, h = btn_body_h - panel_t);
}

module strips() {
  for (r = [0 : 2]) {
    yb = strip_back_y(pad_xy(r * 3)[1]);
    translate([pad_cx - strip_len / 2, yb + 0.01, panel_t + 0.01])
      cube([strip_len, strip_t - 0.02, strip_w]);
  }
}

// The plate rests on the rim (zero-thickness contact); lift it 0.01 mm for
// the shell check so only real overlaps remain.
if (check == "shell")
  intersection() { shell(); translate([0, 0, 0.01]) place() faceplate(); }
else if (check == "buttons")
  intersection() { buttons(); union() { faceplate(); strips(); } }
else if (check == "strips")
  intersection() { strips(); union() { faceplate(); translate([0, 0, 0]) mirror([0, 0, 1])
    translate([0, 0, -(shell_h + panel_t)]) shell(); } }
else {
  color("gold") shell();
  place() { color("white", 0.9) faceplate(); color("lightblue") buttons(); color("red") strips(); }
}
