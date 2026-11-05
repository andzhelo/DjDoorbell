# SPEC — DJ Doorbell, YouTube film (storyline A, 2026-11-05)

GOAL:    a stranger understands in 85 s that this doorbell is played, not pressed, how one press
         gets from the door to the kitchen and the phone, and that one person built it; they want to
         see the repo.
OUTPUT:  HyperFrames, 1920 x 1080, 30 fps.
TIME:    44 bars at 124.016 BPM = 85.16 s = 2555 frames. Bar 1.93548 s (58.06 frames), beat 0.48387 s
         (14.5 frames), 16th 0.12097 s. Every cut sits on a bar or beat line.
LOOK:    two grounds. (1) The footage itself, as shot, no grade, cover-cropped; portrait clips stand in
         a 608-px-wide pillar on a blurred (40 px) and darkened (55 %) copy of themselves, never bars.
         (2) The paper ground of the project page, #f6f7f1, for the diagram only, ink #15170f.
         Accent: the printed box's lime #c8ff2e, on one word per title, the diagram lines, the end card.
         Display: League Gothic 400, uppercase, 150 px (hook/payoff 190 px), line-height 0.92,
         tracking 0.01em, white on footage with a 0 2px 10px 60 % + 0 4px 40px 50 % shadow; black on
         paper. Small text: Inter 700, 40 px. Titles sit in the left 1100 px, bottom band (y 740-960)
         or top band (y 110-330), under a 0 -> 72 % black gradient band 420 px tall; never centred.
         Photos: linear 6-12 % push over their clip. Videos: cut, never moved.
OBJECTS: the pad (lime box, nine clear buttons, three rows amber/cyan/magenta); the inside box (orange,
         grille, knob); the phone page (black, nine coloured circles); the diagram nodes are real
         clips in 12 px lime frames with rounded 18 px corners, joined by 8 px lime lines that draw
         left to right; an iPhone banner (white rounded card, ntfy icon square, "DJ Doorbell",
         "Someone is playing the doorbell", "now").
MOTION:  titles: words blur-in 4 frames, stagger 2 frames, power3.out; they land and hold. Diagram:
         each node scales 0.92 -> 1 in 8 frames (power3.out) as its line finishes drawing (line 12
         frames, power2.inOut). End card: lime field wipes up 12 frames power3.inOut, name words rise
         40 px in 9 frames stagger 2. Photos: scale 1 -> 1.08 linear. Nothing else moves.
SHOTS (bars):
  0-3   HOOK finished_7157 pillar from 3.0 s, its own sound; title at frame 0 "my doorbell has no button"
        (top-left). -> hard cut on bar 4.
  4-7   THE WALL wall_6925 (old button out), p6338 (8.18 V), p6340 (two wires, blue LED), wires_6926
        (the two bare wires). Titles "this is what was in the wall" / "one button. two wires. 8 volts."
        / "so the pair carries 12 V" / "and the music never leaves the box". Music enters on bar 4.
  8-13  HOW IT WORKS paper ground. Nodes appear one per bar with their line: PAD (door clip) ->
        "snaps to the 16th grid" (a 16-step strip, one cell lit per 16th) -> RADIO "30 bytes cross the
        door" -> INSIDE BOX (inside_7122) -> PHONE (phone_7123). Last bar: "nothing else crosses: both
        boxes hold the same samples".
  14-21 BUILD one shot and one line per bar: p6770 coupon "24 mm hole. 2.5 mm plate." / face_6898
        "nine arcade buttons" / solder_6902 "nine wires. one ground." / tl_6911 "24 minutes of
        soldering" / strip_6916 "three strips, lit from the side" / p7100 "first light" / assy_6923a
        "the shell" / hero_6921 "first fit".
  22-23 FIVE BOARDS p7017 then p7121, "5 boards. 1 survived." (magenta accent). Music thins.
  24-27 INSTALL p7107 "breaker off" / p7111 "230 V in, 12 V out" / shellwall_6927 "12 V on the pair" /
        box_6915 "on the wall".
  28-33 PAYOFF door_7119 pillar with its own sound (from 9.6 s), bar 31 the frame splits: inside_7122
        pillar joins on the right with its own sound, title "the inside box answers, 30 ms behind".
  34-37 THE BOX AND THE PHONE p7155 "its own printed box" / p7156 "a knob for the volume" /
        phone_7123 pillar + settings card image "quiet after 22:00" / banner mock "and a message to
        my phone".
  38-39 CLAIM finished_7157 pillar from 12 s, "a doorbell you play, not press" (190 px, top-left).
  40-43 END CARD lime field, "DJ Doorbell", "github.com/andzhelo/DjDoorbell", "open source · esp32 ·
        3d printed"; chime at bar 40 beat 1; music fades 2 s.
SOUND:   the engine's own 44-bar performance (make_track.py), 124 BPM; real door audio in the hook
         and the payoff with a 90 Hz high-pass; music bars 4-27 and 34-43; chime on the end card.
         Target -14 LUFS integrated, -1 dBTP in the delivery transcode.
BANNED:  dark void with floating UI; glow, particles; cream + serif; centred titles; fades on every
         cut; three cards in a row; a docked logo; titles that drift after landing; black side bars.
QA:      frame 0 (hand on the pad, title legible, thumbnail); bar 8 (paper ground, first node only);
         bar 13 (all five nodes, lines complete); bar 31 (two pillars, both faces of the project);
         bar 36 (banner readable); bar 40 (end card lands); every title at 360 px width.
