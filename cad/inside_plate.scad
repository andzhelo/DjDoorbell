// Inside unit plate: speaker grille, retaining ring on the back, optional
// volume knob hole. Printed face-down for the finish.
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

      // speaker retaining ring
      translate([spk_cx, spk_cy, panel_t])
        difference() {
          cylinder(d = spk_ring_od, h = spk_ring_h);
          translate([0, 0, -1]) cylinder(d = spk_d + 0.4, h = spk_ring_h + 2);
        }
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

    // speaker mounting screws, if the frame has holes
    if (spk_hole_pcd > 0)
      for (a = [0 : 90 : 270])
        translate([spk_cx + spk_hole_pcd / 2 * cos(a + spk_hole_angle),
                   spk_cy + spk_hole_pcd / 2 * sin(a + spk_hole_angle), -1])
          cylinder(d = spk_hole_d, h = panel_t + spk_ring_h + 2, $fn = 32);

    // volume knob
    if (pot_hole_d > 0)
      translate([pot_xy[0], pot_xy[1], -1]) cylinder(d = pot_hole_d, h = panel_t + rib_h + 2);

    // corner screws, countersunk from the front
    for (p = corner_xy) translate([p[0], p[1], 0]) {
      translate([0, 0, -1]) cylinder(d = screw_clear_d, h = panel_t + rib_h + 2);
      translate([0, 0, -0.01]) cylinder(d1 = screw_head_d, d2 = screw_clear_d,
                                        h = (screw_head_d - screw_clear_d) / 2);
    }
  }
}

inside_plate();
