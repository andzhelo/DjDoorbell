// Button hole test coupon: one 36 x 36 tile per candidate diameter, one row
// per panel thickness. Snap a button into each; the tile where it clicks in
// firmly without forcing sets btn_hole_d and panel_t in params.scad.
//
// Print hole-up (text on top), same material and settings as the faceplate.
include <params.scad>

// 36 mm tile: the hole's lowest point is then 5.9 mm from the edge, so the
// label row (1.8-5.0 mm) never runs into the hole. (30 mm did.)
tile = 36;
gap  = 4;

for (j = [0 : len(coupon_thick) - 1], i = [0 : len(coupon_holes) - 1]) {
  d = coupon_holes[i];
  t = coupon_thick[j];
  translate([i * (tile + gap), j * (tile + gap), 0]) {
    difference() {
      cube([tile, tile, t]);
      translate([tile / 2, tile / 2, -1]) cylinder(d = d, h = t + 2);
    }
    // label "hole / thickness": embossed on the top face, below the hole
    dl = (d == floor(d)) ? str(d, ".0") : str(d);
    tl = (t == floor(t)) ? str(t, ".0") : str(t);
    translate([2, 1.8, t])
      linear_extrude(0.6)
        text(str(dl, "/", tl), size = 3.0, font = "Liberation Sans:style=Bold");
  }
}
