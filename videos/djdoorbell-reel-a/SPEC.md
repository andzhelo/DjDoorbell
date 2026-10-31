GOAL:    A stranger scrolling past stops on the hand hitting the pads, understands "a doorbell that is a drum machine"
         in one second, stays for the build because every cut lands on the beat, and leaves wanting one.
OUTPUT:  HyperFrames, 1080 x 1920, 30 fps (Instagram Reel).
TIME:    18 bars at 124.016 BPM. Bar = 1.93523 s = 58.06 frames; beat = 0.48381 s = 14.51 frames; 16th = 0.12095 s.
         Total 34.83 s = 1045 frames. The music is the doorbell's own engine playing a scripted performance
         (assets/audio/track.wav); the first press of the film IS the first kick at frame 0.
LOOK:    Ground: the footage itself, full bleed, no border. Where a landscape clip is shown it is cover-cropped to 9:16
         with the subject placed by object-position, never letterboxed. Ink: white #ffffff. One accent: the printed
         box's lime #c8ff2e, allowed on: one word per title, the 16-step grid strip, the end card mark.
         Row colours amber #ffb000 / cyan #22e6ff / magenta #ff2bd6 only on the pad diagram and the step grid.
         Display: League Gothic (condensed, bundled) at 150 px, line 0.92, tracking 0.01em, uppercase, white, with a
         4 px lime underline bar on the accent word; small lines Inter 700 at 44 px. Everything inside 168-1330 px vertically.
         Medium: phone footage, graded once (contrast +8 %, warmth +4 %, 2 % grain) through media-treatment where used;
         no CSS filters. Type sits on a 55 % black-to-transparent gradient band only when the plate behind is bright.
OBJECTS: the lime box on the wall; the hand; the nine clear buttons lit amber/cyan/magenta; the old white Nisko button;
         the multimeter showing 8.18 V; the parts bags; the ESP32-S3 in a hand; the WS2812 strip; the lime faceplate
         being soldered; the strip lighting for the first time; the WROOM on its breakout for the link test;
         tape-wrapped parts; the IRM-10-12; the phone with the nine coloured pads.
MOTION:  Cuts on the 16th grid: every seam sits on a beat (14.51 f) or an 8th. Footage moves inside the frame:
         one slow push (scale 1.0 to 1.08, linear) per held shot, no push on shots under 1 s.
         Titles: per-word blur 6 px to 0 over 4 frames, 2 frames apart; land dead still; exit with the cut (hard).
         Chapter lines hold 0.4-0.6 s; claims hold 0.5 s + 0.3 s a word. The step-grid strip advances one cell per 16th.
         Payoff shots play at normal speed; the build montage at 1 shot per beat or per bar as listed.
SHOTS:   (f = frame; bar b spans [58.06*b, 58.06*(b+1)))
         b0  f0-58     HOOK  door_7119 from 7.9 s (hand already on the pad, amber flash at 8.1 s). Title "MY DOORBELL IS A
                      DRUM MACHINE" already on frame 0, "DRUM MACHINE" in lime. Grid strip starts stepping.
         b1-2 f58-174 STARTED  wall_6925 (old button) 1 bar, p6338 (8.18 V meter, push) half bar, p6339 (chime box) half bar.
                      Title "it started with this" / "one button. 8 volts."
         b3-4 f174-290 ORDERED  p6807 (bags) 1 beat, p7017 (S3 in hand) 1 beat, p7018 (strip) 1 beat, p6819 (buck) 1 beat,
                      p6703 (one arcade button) 2 beats, p6704 (breadboard) 2 beats. Title "so I ordered everything".
         b5-9 f290-581 BUILD  fit_6837 coupon 1 bar · face_6898 faceplate 1 bar · solder_6902 1 bar · tl_6912 time-lapse
                      1 bar · led_6716 LEDs first light 1 bar. Titles one per bar: "24 mm hole, 2.5 mm plate" ·
                      "nine arcade buttons" · "nine wires. one ground." · "24 minutes of soldering" · "first light".
         b10 f581-639 FIVE BOARDS  p7017 cut hard to p7121 (tape). Title "5 boards. 1 survived." magenta on "5".
         b11-12 f639-755 INSTALL  p7107 (old box) half bar, p7111 (IRM in hand) half bar, shellwall_6927 1 bar.
                      Title "breaker off. 12 V on the pair."
         b13-15 f755-929 PAYOFF  door_7119 from 9.5 s, full bleed, no title for the first bar; then "EVERY VISITOR IS A DJ"
                      at b14; inside_7122 for the last half bar ("...and the chime inside follows").
         b16 f929-987 PHONE  phone_7123 from 4.0 s. Title "or from the phone".
         b17 f987-1045 END CARD  lime field wipes up from the bottom on the beat, "DJ DOORBELL" in black League Gothic,
                      "github.com/andzhelo/DjDoorbell" Inter 44 px, holds to the end; chime plays.
BANNED:  black void with floating elements; glow, bloom, particles; purple/blue gradients; cream-serif-mono look; every
         title fading and sliding up; letterboxed landscape clips; a docked logo; captions below 1330 px; crossfades
         between worlds (hard cuts on the beat only, plus the one lime wipe into the end card).
QA:      f0 (hand on pad + title legible, nothing black), f58 (cut lands on the beat), f290, f581 (five boards legible),
         f755 (payoff clean, no title), f987 (wipe), f1044 (end card complete).
