// Button hole test coupon: one 36 x 36 tile per candidate diameter, at the
// real panel thickness. Snap a button into each; the one that clicks in
// firmly without forcing sets btn_hole_d in params.scad.
//
// Print hole-up (text on top), same material and settings as the faceplate.
include <params.scad>

// 36 mm tile: the hole's lowest point is then 5.9 mm from the edge, so the
// label row (1.8-5.0 mm) never runs into the hole. (30 mm did.)
tile = 36;
gap  = 4;

for (i = [0 : len(coupon_holes) - 1]) {
  d = coupon_holes[i];
  translate([i * (tile + gap), 0, 0]) {
    difference() {
      cube([tile, tile, panel_t]);
      translate([tile / 2, tile / 2, -1]) cylinder(d = d, h = panel_t + 2);
    }
    // label: embossed on the top face, below the hole
    label = (d == floor(d)) ? str(d, ".0") : str(d);
    translate([2, 1.8, panel_t])
      linear_extrude(0.6)
        text(label, size = 3.2, font = "Liberation Sans:style=Bold");
  }
}
