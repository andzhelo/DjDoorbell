// Inside unit plate: speaker grille, four standoff bosses (M3 inserts) for the
// square speaker frame, volume knob hole. Printed face-down for the finish.
include <inside_params.scad>

module inside_plate() {
  difference() {
    union() {
      linear_extrude(panel_t) rounded_rect(plate_w, plate_h, corner_r);

      // locating rib inside the shell wall, notched around the corner bosses
      translate([0, 0, panel_t])
        linear_extrude(rib_h)
          difference() {
            offset(delta = -(wall_t + fit_clear)) rounded_rect(plate_w, plate_h, corner_r);
            offset(delta = -(wall_t + fit_clear + 2)) rounded_rect(plate_w, plate_h, corner_r);
            for (p = corner_xy) translate(p) circle(d = boss_d + 2 * fit_clear + 1);
          }

      // speaker standoff bosses at the frame's four holes
      for (sx = [-1, 1], sy = [-1, 1])
        translate([spk_cx + sx * spk_hole_off, spk_cy + sy * spk_hole_off, panel_t - 0.01])
          cylinder(d = spk_boss_d, h = spk_boss_h + 0.01);
    }

    // grille: hex-packed holes over the cone opening
    translate([spk_cx, spk_cy, -1])
      let (n = ceil(spk_cone_d / 2 / grille_pitch) + 1)
      for (r = [-n : n], c = [-n : n]) {
        x = (c + (r % 2 == 0 ? 0 : 0.5)) * grille_pitch;
        y = r * grille_pitch * 0.866;
        if (sqrt(x * x + y * y) <= spk_cone_d / 2)
          translate([x, y, 0]) cylinder(d = grille_hole_d, h = panel_t + 2, $fn = 24);
      }

    // heat-set insert holes in the speaker bosses (blind, from the back)
    for (sx = [-1, 1], sy = [-1, 1])
      translate([spk_cx + sx * spk_hole_off, spk_cy + sy * spk_hole_off, panel_t + spk_boss_h - spk_insert_h])
        cylinder(d = insert_hole_d, h = spk_insert_h + 1, $fn = 32);

    // volume knob
    if (pot_hole_d > 0)
      translate([pot_xy_set[0], pot_xy_set[1], -1]) cylinder(d = pot_hole_d, h = panel_t + rib_h + 2);

    // corner screws, countersunk from the front
    for (p = corner_xy) translate([p[0], p[1], 0]) {
      translate([0, 0, -1]) cylinder(d = screw_clear_d, h = panel_t + rib_h + 2);
      translate([0, 0, -0.01]) cylinder(d1 = screw_head_d, d2 = screw_clear_d,
                                        h = (screw_head_d - screw_clear_d) / 2);
    }
  }
}

inside_plate();
