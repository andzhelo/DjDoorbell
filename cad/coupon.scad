// Button hole test coupon: one 30 x 30 tile per candidate diameter, at the
// real panel thickness. Snap a button into each; the one that clicks in
// firmly without forcing sets btn_hole_d in params.scad.
//
// Print hole-up (text on top), same material and settings as the faceplate.
include <params.scad>

tile = 30;
gap  = 4;

for (i = [0 : len(coupon_holes) - 1]) {
  d = coupon_holes[i];
  translate([i * (tile + gap), 0, 0]) {
    difference() {
      cube([tile, tile, panel_t]);
      translate([tile / 2, tile / 2, -1]) cylinder(d = d, h = panel_t + 2);
    }
    // label: embossed on the top face, in a corner clear of the flange
    translate([2, 1.5, panel_t])
      linear_extrude(0.6)
        text(str(d), size = 3.2, font = "Liberation Sans:style=Bold");
  }
}
