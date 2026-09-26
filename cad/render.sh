#!/bin/sh
# Render every part to cad/stl/ and a preview PNG to cad/preview/.
#   cad/render.sh            all parts
#   cad/render.sh faceplate  one part
set -e
cd "$(dirname "$0")"
mkdir -p stl preview
parts=${*:-"coupon faceplate shell"}
for p in $parts; do
  openscad --backend=manifold -o "stl/$p.stl" "$p.scad"
  openscad --backend=manifold -o "preview/$p.png" --imgsize=1200,900 \
    --autocenter --viewall --projection=o "$p.scad" 2>/dev/null || true
  echo "$p: $(du -h "stl/$p.stl" | cut -f1)"
done
