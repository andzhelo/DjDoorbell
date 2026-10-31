# Review — DJ Doorbell reel, storyline A (2026-10-31)

## Creative calls
- Storyline A ("my doorbell is a drum machine", result first, cut on the beat). B ("every visitor becomes a DJ",
  a stranger at the door) needs a shot we do not have; C ("I killed 5 boards") is a maker confession that reaches
  the wow late. Both are in STORYLINES.md.
- Music is the doorbell's own engine (tools/simulator.py) playing a scripted 19-bar performance at 124 BPM, so the
  first press in frame 0 is the first kick and every cut sits on a bar or beat line. No catalog music, no licence.
- Look: footage as shot, the printed box's lime #c8ff2e as the single accent (one word per title, the end card),
  League Gothic condensed titles, gradient band under the type only. Photos get a linear 6-12 % push; videos are
  cut, not moved. Landscape clips are cover-cropped to 9:16 with the subject placed by object-position.
- Hook: door footage from 7.9 s with the title already on frame 0 (thumbnail safe). Payoff: two bars of the door
  with no title for the first bar, then "every visitor is a DJ". End card: lime field wipes up on the bar line,
  name + GitHub link hold 3.9 s, the chime plays under it.

## Measured
- `npx hyperframes check`: ok; warnings kept: `nested_media_start_basis_ambiguous` (nested clips timed locally on
  purpose, one scene per beat) and `timeline_track_too_dense` (5-6 clips on a lane in the build scene).
- Review sheets (beats, seams, phone width): every title legible at 360 px; all nine seams are hard cuts on the
  beat; frame 0 is the hand on the pad with the title.
- Sound was generated and measured, not heard: 37.3 s track, peak-normalised to 0.89, chime at bar 17 beat 1.

## Fixed during review
- Titles vanished under scaled photos: the root's `data-motion-blur="on"` captured zooming elements above the
  type. Removed. `<img>` clips were also laid out by the runtime as their own 3-s media; photos are now divs
  with a background image.
- End card lines were under the 2 s read floor: end card extended to two bars (track regenerated to 19 bars).
- Phone title moved to the top band so it does not sit over the phone's pads.

## Weakest shots / with more time
- "first light" (bar 10) uses the bench strip clip; the LEDs are small in frame. A close-up of the strip lighting
  for the first time would be stronger.
- "5 boards. 1 survived." is told with the board-in-hand photo and the taped module; a photo of an actual dead
  board would land harder.
- The inside unit (IMG_7122) is not in the cut; a half-bar of it after the payoff would close the "inside follows"
  idea. Dropped to keep 37 s.
- No voice: the user asked for a hook and a build story; titles carry it. Captions are not needed (no speech).

## v3 (loop + real hook sound)
- Hook keeps the door clip's own audio (gain 1.6); the generated beat enters at bar 2 with a 0.25 s fade-in.
- Film is exactly 19 bars (36.77 s). End card holds 1.5 bars, then a half-bar tail of the door clip (6.93-7.9 s)
  with the hook title already on, so the loop lands on frame 0 without a jump. `line_unread` on the tail title is
  kept on purpose: it continues into the hook.
- The lime field is a timed clip now; as an untimed div it dropped out of the last frames of the v2 render and
  left the end-card text over the footage.
