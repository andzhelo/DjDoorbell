// Faceplate: 3 x 3 button grid on top, speaker grille in the bay below.
// Printed face-down for the finish. Buttons snap in from the front; the
// speaker sits in the retaining ring on the back, held by the shell.
include <params.scad>

module faceplate() {
  difference() {
    union() {
      linear_extrude(panel_t) rounded_rect(plate_w, plate_h, corner_r);

      // locating rib: fits inside the shell wall
      translate([0, 0, panel_t])
        linear_extrude(rib_h)
          difference() {
            offset(delta = -(wall_t + fit_clear)) rounded_rect(plate_w, plate_h, corner_r);
            offset(delta = -(wall_t + fit_clear + 2)) rounded_rect(plate_w, plate_h, corner_r);
          }

      // speaker retaining ring
      translate([bay_cx, bay_cy, panel_t])
        difference() {
          cylinder(d = spk_d + 4, h = spk_ring_h);
          translate([0, 0, -1]) cylinder(d = spk_d + 0.4, h = spk_ring_h + 2);
        }
    }

    // button holes
    for (i = [0 : 8]) translate([pad_xy(i)[0], pad_xy(i)[1], -1])
      cylinder(d = btn_hole_d, h = panel_t + rib_h + 2);

    // speaker grille: hex-packed holes inside grille_d
    translate([bay_cx, bay_cy, -1])
      for (r = [-4 : 4], c = [-4 : 4]) {
        x = (c + (r % 2 == 0 ? 0 : 0.5)) * grille_pitch;
        y = r * grille_pitch * 0.866;
        if (sqrt(x * x + y * y) <= grille_d / 2)
          translate([x, y, 0]) cylinder(d = grille_hole_d, h = panel_t + 2, $fn = 24);
      }

    // corner screws, countersunk from the front
    for (p = corner_xy) translate([p[0], p[1], 0]) {
      translate([0, 0, -1]) cylinder(d = screw_clear_d, h = panel_t + rib_h + 2);
      translate([0, 0, -0.01]) cylinder(d1 = screw_head_d, d2 = screw_clear_d,
                                        h = (screw_head_d - screw_clear_d) / 2);
    }
  }
}

faceplate();
