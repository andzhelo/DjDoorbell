---
workflow: general-video
flow: automation
storyboard: no
message: "A doorbell you play, not press — built from scratch, one build day at a time"
destination: youtube + instagram-reel
aspect: 1920x1080 (YouTube) and 1080x1920 (Reel, second composition `reel.html`)
language: en
length: YouTube ~2:45 build montage; Reel ~39 s (20 bars at 124 BPM)
audience: makers / DIY electronics viewers; the Reel is the hook, the YouTube cut is the build story
angle: build-diary montage, chapter cards, no narration (first draft — narration/install footage to be added after install day)
---

## Intent

Two first-draft edits from the footage shot so far (wall survey → bench → link test →
enclosure → soldering → assembly → first fit at the wall). Both will be revised after the
replacement ESP32-S3 arrives and the unit is installed. No voiceover exists yet; the cuts
carry on-screen chapter text and the project's own audio. Tone: confident maker build-log,
percussive, cut on the beat of the doorbell's own loop.

## Assets

- assets/video/*.mp4 — trimmed, silent H.264 excerpts of the owner's phone footage
  (source: /Users/andzhelo/Desktop/djdorbel_videos). Landscape 1920x1080 unless named
  bench_/led_/link_6812/overhead_/strip_vid/tl_6912 (1080x1920 vertical).
- assets/video/tl_*.mp4 — time-lapses (6911 x40, 6912 x120, 6916 x15).
- assets/photo/*.jpg — wall survey, parts, coupon, boards (downscaled to 2000 px).
- assets/audio/00_loop.wav — the doorbell's 4-bar bed, 124.016 BPM, 7.7410 s, loops seamlessly.
- assets/audio/doorbell-house-demo.wav — 14 s demo performance; 11_chime.wav — plain chime.
- assets/audio/01_kick / 02_clap / 03_hat / 04_stab_am / 08_pluck / 09_riser — the pad one-shots, used as cut SFX.

## Customizations

- Music bed = the project's own loop (no licensed music needed); cuts land on its 124 BPM grid
  (beat 0.4838 s, bar 1.9352 s).
- Pad one-shots as SFX on hard cuts; 11_chime on the end card.
- Decorative motif: the 3x3 pad grid (rows amber / cyan / magenta, as in the firmware) pulsing on the beat.
- Vertical footage inside the 16:9 cut sits in a centred pillar over a blurred copy of itself.

## Notes

- Decisions made autonomously (user: "at least to start, we will update it"): route general-video,
  flow automation, storyboard no, lengths above, lime accent from the printed enclosure.
- GitHub: github.com/andzhelo/DjDoorbell (end card).
- Project audio files live outside git (data/preview is gitignored) — copies staged in assets/audio.
