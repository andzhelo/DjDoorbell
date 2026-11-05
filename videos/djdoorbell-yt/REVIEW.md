# Review — DJ Doorbell, YouTube film, storyline A (2026-11-05)

## Creative calls
- Storyline A ("you play it, you don't press it"): result first, then one press followed from the pad to the
  phone, the build fast in the middle, the finished doorbell and the new features at the end. B (five dead
  boards) and C (door and kitchen split) are in STORYLINES.md.
- 16:9 from phone footage: landscape clips are cut full-bleed; the four 9:16 clips stand in a 608 px pillar
  on a blurred, darkened copy of themselves made with ffmpeg (no CSS filters). Single pillars stand at the
  right, so the title owns the left 1000 px; the payoff splits into two pillars touching at the centre.
- Music is the doorbell's own engine (tools/simulator.py) playing a scripted 44-bar performance
  (make_track.py): every cut sits on the engine's grid. The hook and the payoff use the real door audio
  (90 Hz high-pass); the track enters at bar 4 and returns at bar 34, each on a bar line.
- "How it works" is the one diagram that grows: five nodes, one per bar, joined by lines that draw in; the
  pad, the inside box and the phone are real clips in lime frames, the engine node is a 16-cell grid with
  one cell lit per 16th in time with the music, the radio node is the number 30 (bytes per packet). Paper
  ground #f6f7f1 from the project page, so the film has two worlds, not one dark canvas.
- No voice: titles carry it (the user did not choose a narration). The script can be read over the same cut
  later; no title repeats what a voice would say word for word.

## Measured
- `npx hyperframes check`: ok. Warnings kept: `nested_media_start_basis_ambiguous` (nested clips timed
  locally on purpose), `timeline_track_too_dense` (one scene per chapter), one `contrast_aa_failure` at
  2.98:1 on a white word over the door footage in the payoff (shade deepened to 82 %; the word sits on it).
- Snapshots at 0, 9, 16.5, 21.3, 22, 26.5, 31, 44, 50, 56, 58, 61, 67, 72.5, 75, 80 s: frame 0 is the hand on
  the pad with the title legible (thumbnail safe); the diagram reads at every stage; the split payoff shows
  the pad and the orange box side by side; the banner reads; the end card lands.
- Sound was generated and measured, not heard. The render came out at -13.9 LUFS with the real door
  audio 4-7 dB under the music (hook bars -17..-22, payoff bars -17..-23, music -14 flat). Delivery
  pass (ffmpeg, video copied): hook x1.8, music x0.8, payoff x2.0, true-peak limiter at -1 dB, auto-level
  off: -14.2 LUFS integrated, peak -0.7 dBFS, per-bar RMS -13..-21 (std 2.4 over bars 0-41).

## Render notes
- The two door clips are iPhone HLG (bt2020 / arib-std-b67 tags). The default render took the HDR path
  (18 GB of 16-bit frames) and timed out on frame capture; `--sdr` renders them as SDR in 20 min.
- The first SDR render encoded every frame, then the mix stage sat idle past the 10-min encoder watchdog
  (runtime high-pass fx chains on two long clips). The 90 Hz high-pass is now baked into the clips'
  audio with ffmpeg, no fx chain at render time, and `FFMPEG_ENCODE_TIMEOUT_MS=2400000` as a margin.
- Delivery file: renders/djdoorbell-youtube.mp4 (1920x1080, 30 fps, H.264 CRF 16, AAC 256k, 83 MB).

## Fixed during review
- Node videos inside timed node frames: the lint refuses a timed video inside a timed wrapper, and an
  untimed video. The frames are now untimed divs revealed by GSAP; the videos inside are the timed clips.
- Titles in pillar scenes ran under the pillar: single pillars moved to the right (x 1150).
- The "pad" node used the clip from 6 s and walked indoors at 13 s: now from 1.5 s. The clip's indoor
  half (the orange box playing) became the inside-box node and the payoff's right pillar.
- The accent on paper (#7fb800) failed AA at 2.2:1: text accent is #5a8a00, lines stay #7fb800.
- Last diagram line was under its read floor: shortened to "no audio crosses. same samples, both sides".

## Weakest shots / with more time
- The payoff's two pillars are two different performances (install day at the door, today's clip inside),
  so only the door audio plays. A two-phone recording of one performance would let both sound.
- No visitor reaction: the only hands are the owner's. One shot of a stranger at the door would lift the
  hook and the claim.
- A voice. The cut is paced for titles; a 150-word narration recorded on a phone would make the middle
  (wall, diagram, build) land harder on YouTube.
