// Faceplate mounted on the shell, for checking fit. Not for printing.
//   interference = true  renders only where the two parts overlap (must be empty)
include <params.scad>
use <faceplate.scad>
use <shell.scad>

interference = false;

module placed_faceplate() {
  // plate back sits on the shell rim; front face up
  translate([0, 0, shell_h + panel_t]) mirror([0, 0, 1]) faceplate();
}

// The plate rests on the rim (zero-thickness contact); lift it 0.01 mm for the
// check so only real overlaps remain.
if (interference) intersection() { shell(); translate([0, 0, 0.01]) placed_faceplate(); }
else { color("gold") shell(); color("white", 0.9) placed_faceplate(); }
