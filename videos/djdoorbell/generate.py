#!/usr/bin/env python3
"""Generate index.html (YouTube 16:9) here and ../djdoorbell-reel/index.html (Instagram 9:16).

Every cut sits on the doorbell's own grid: 124.016 BPM, 4-bar loop = 7.741 s.
Edit the SHOT lists below and re-run; or edit the HTML directly in Studio and
stop running this script (it overwrites both files).
"""
from pathlib import Path

BPM = 124.016
BEAT = 60 / BPM            # 0.48381 s
BAR = 4 * BEAT             # 1.93523 s
LOOP = 4 * BAR             # 7.74094 s (00_loop.wav is 7.74095 s)

HERE = Path(__file__).parent

BG = "#0d0f0a"
FG = "#f2f3ea"
LIME = "#c8ff2e"
AMBER = "#ffb000"
CYAN = "#22e6ff"
MAGENTA = "#ff2bd6"
MUTED = "rgba(242,243,234,0.62)"

LOOP_SRC = "assets/audio/00_loop.wav"
# name: (file, duration, track)
SFX = {
    "kick": ("assets/audio/01_kick.wav", 0.52, 11),
    "clap": ("assets/audio/02_clap.wav", 0.42, 12),
    "hat": ("assets/audio/03_hat.wav", 0.055, 13),
    "stab": ("assets/audio/04_stab_am.wav", 0.62, 14),
    "pluck": ("assets/audio/08_pluck.wav", 0.34, 15),
    "riser": ("assets/audio/09_riser.wav", 1.10, 16),
    "chime": ("assets/audio/11_chime.wav", 2.00, 17),
}


def f(x):
    return f"{x:.3f}"


# ---------------------------------------------------------------- shot lists
# (kind, src, start_bar, end_bar, media_start)  kind: v = video, p = photo
YT_SHOTS = [
    # 00 cold open
    ("v", "hero_6921", 0, 4, 4.0),
    # 01 the wall
    ("p", "IMG_6335", 4, 6, 0),
    ("p", "IMG_6336", 6, 7, 0),
    ("p", "IMG_6338", 7, 8, 0),
    ("p", "IMG_6339", 8, 9, 0),
    ("v", "wall_6925", 9, 12, 1.5),
    # 02 bench
    ("p", "IMG_6807", 12, 13.5, 0),
    ("p", "IMG_6703", 13.5, 14.5, 0),
    ("v", "bench_6699_wide", 14.5, 16.5, 4.0),
    ("v", "led_6716_wide", 16.5, 18.5, 3.0),
    ("p", "IMG_6704", 18.5, 19.5, 0),
    ("p", "IMG_6814", 19.5, 20.5, 0),
    ("p", "IMG_6819", 20.5, 21.5, 0),
    ("v", "power_6885", 21.5, 22, 1.0),
    # 03 the gate (link test)
    ("v", "link_6813", 22, 24, 0.0),
    ("v", "link_6813", 24, 25, 4.0),
    ("v", "link_6813", 25, 27.5, 8.0),
    ("v", "link_6813", 27.5, 30, 13.0),
    # 04 enclosure
    ("p", "IMG_6717", 30, 31, 0),
    ("p", "IMG_6770", 31, 32, 0),
    ("v", "shell_6837", 32, 36, 2.0),
    ("v", "fit_6837", 36, 38.5, 2.0),
    ("v", "face_6898", 38.5, 40, 1.0),
    # 05 soldering
    ("v", "power_6884", 40, 42, 0.0),
    ("v", "solder_6899", 42, 46, 20.0),
    ("v", "tl_6912_wide", 46, 52, 0.0),
    ("v", "tl_6911", 52, 56, 0.0),
    ("v", "wire_6917", 56, 58, 4.0),
    # 06 assembly
    ("v", "assy_6923a", 58, 62, 4.0),
    ("v", "assy_6923a", 62, 64, 36.0),
    ("v", "hero_6921", 64, 66, 0.0),
    ("v", "hero_6921", 66, 68, 11.0),
    # 07 the wall, again
    ("v", "wires_6926", 68, 70.5, 1.0),
    ("v", "shellwall_6926", 70.5, 73, 1.0),
    ("v", "shellwall_6927", 73, 76, 1.0),
    # 76-82 end card over a dark hold (no footage)
]
YT_BARS = 82

# chapter cards: (id, start_bar, number, title, line)
YT_CARDS = [
    ("c1", 4, "01", "THE WALL", "two wires · 8 V AC · one dumb button"),
    ("c2", 12, "02", "THE BENCH", "ESP32-S3 · nine arcade buttons · one speaker"),
    ("c3", 22, "03", "THE GATE", "ESP-NOW through a closed door"),
    ("c4", 30, "04", "ENCLOSURE", "Ø24 mm holes · 30 mm pitch · printed in lime"),
    ("c5", 40, "05", "SOLDERING", "nine switches · nine wires · 27 pixels"),
    ("c6", 58, "06", "ASSEMBLY", "it exists."),
    ("c7", 68, "07", "THE WALL, AGAIN", "12 V on the pair · install day next"),
]

# (kind, src, start_bar, end_bar, media_start, caption)   reel: 2 bars per shot
REEL_SHOTS = [
    ("v", "hero_6921", 0, 2, 6.0, None),
    ("v", "wall_6925", 2, 4, 6.0, "the old one:<br>a button."),
    ("v", "bench_6699", 4, 6, 4.0, "ESP32-S3<br>+ 9 arcade buttons"),
    ("v", "led_6716", 6, 8, 3.0, "27 pixels"),
    ("v", "link_6812", 8, 10, 8.5, "link test<br>through the door ✓"),
    ("v", "tl_6912", 10, 12, 0.0, "24 min of soldering"),
    ("v", "overhead_811847", 12, 14, 2.0, None),
    ("v", "face_6897", 14, 16, 0.5, "every press snaps<br>to a 16th-note grid"),
    ("v", "hero_6921", 16, 18, 12.0, "124 BPM · A minor"),
    ("v", "shellwall_6927", 18, 20, 3.0, "part 2:<br>install day"),
]
REEL_BARS = 20

VERTICAL = {"bench_6699", "led_6716", "link_6812", "overhead_811847", "strip_vid", "tl_6912"}


# ------------------------------------------------------------------ helpers
def video_clip(i, src, s, e, ms, track=1, z=1, extra_class=""):
    return (
        f'<video id="v{i:02d}" class="clip vid {extra_class}" src="assets/video/{src}.mp4" '
        f'data-start="{f(s)}" data-duration="{f(e - s)}" data-media-start="{f(ms)}" '
        f'data-track-index="{track}" muted playsinline style="z-index:{z}"></video>'
    )


def photo_clip(i, src, s, e, track=2, z=1):
    return (
        f'<div id="p{i:02d}" class="clip photo" data-start="{f(s)}" data-duration="{f(e - s)}" '
        f'data-track-index="{track}" style="z-index:{z}"><img id="p{i:02d}-bg" class="pbg" data-layout-allow-overflow '
        f'src="assets/photo/{src}.jpg" alt=""><img id="p{i:02d}-img" class="kb" data-layout-allow-overflow '
        f'src="assets/photo/{src}.jpg" alt=""></div>'
    )


def bed(total, track=10, vol=0.45, fade=2.0):
    out = []
    n = int(total // LOOP) + 1
    for k in range(n):
        s = k * LOOP
        d = min(LOOP - 0.004, total - s)  # 4 ms gap so rounded windows never overlap
        if d <= 0.05:
            break
        auto = ""
        if s + LOOP >= total - 0.01:  # last copy: fade out
            fs = max(0.0, d - fade)
            auto = (
                f' data-automation=\'{{"version":1,"lanes":[{{"target":"volume","points":['
                f'{{"t":0,"v":1}},{{"t":{f(fs)},"v":1}},{{"t":{f(d)},"v":0}}]}}]}}\''
            )
        out.append(
            f'<audio id="bed{k:02d}" src="{LOOP_SRC}" data-start="{f(s)}" data-duration="{f(d)}" '
            f'data-track-index="{track}" data-volume="{vol}"{auto}></audio>'
        )
    return out


def sfx(i, name, t, vol=0.7):
    src, dur, track = SFX[name]
    return (
        f'<audio id="sfx{i:02d}-{name}" src="{src}" data-start="{f(t)}" data-duration="{f(dur)}" '
        f'data-track-index="{track}" data-volume="{vol}"></audio>'
    )


FONT_CSS = """
      .display { font-family: "Archivo Black", "League Gothic", sans-serif; font-weight: 400; }
      .mono { font-family: "JetBrains Mono", monospace; font-weight: 700; }
"""

HUD_HTML = """
      <div id="hud" class="clip hud" data-start="0" data-duration="{dur}" data-track-index="4" style="z-index:6">
        <div id="hud-grid" class="padgrid">
          <i class="pad r1"></i><i class="pad r1"></i><i class="pad r1"></i>
          <i class="pad r2"></i><i class="pad r2"></i><i class="pad r2"></i>
          <i class="pad r3"></i><i class="pad r3"></i><i class="pad r3"></i>
        </div>
        <div id="hud-steps" class="steps">{steps}</div>
        <div id="hud-label" class="mono hudlabel">124.016 BPM · A MINOR · 16TH GRID</div>
      </div>
"""


def hud(dur, ncols=16):
    steps = "".join('<b class="step"></b>' for _ in range(ncols))
    return HUD_HTML.format(dur=f(dur), steps=steps)


# JS shared by both compositions: beat chrome driven from one grid.
HUD_JS = """
      // ---- rhythm chrome: one tempo grid drives the pad grid + step bar ----
      const BEAT = %(beat).6f, BAR = %(bar).6f, TOTAL = %(total).3f;
      const pads = gsap.utils.toArray("#hud-grid .pad");
      const steps = gsap.utils.toArray("#hud-steps .step");
      const nBeats = Math.floor(TOTAL / BEAT);
      for (let b = 0; b < nBeats; b++) {
        const t = b * BEAT;
        // kick on 1 & 3 lights row 1, clap on 2 & 4 lights row 3, row 2 walks.
        const which = (b %% 4 === 0) ? b %% 3 : (b %% 2 === 1) ? 6 + (b %% 3) : 3 + (b %% 3);
        tl.fromTo(pads[which], { scale: 1.45, opacity: 1 }, { scale: 1, opacity: 0.35, duration: BEAT * 0.9, ease: "power3.out", immediateRender: false, overwrite: "auto" }, t);
      }
      const nSteps = Math.floor(TOTAL / (BEAT / 4));
      for (let s = 0; s < nSteps; s++) {
        const t = s * (BEAT / 4);
        tl.fromTo(steps[s %% 16], { opacity: 1, scaleY: 1.6 }, { opacity: 0.25, scaleY: 1, duration: BEAT / 4 * 0.95, ease: "none", immediateRender: false, overwrite: "auto" }, t);
      }
"""


def ken_burns_js(photos):
    js = []
    for i, (s, e) in photos:
        d = e - s
        sx = 1.0 if i % 2 else 1.08
        ex = 1.08 if i % 2 else 1.0
        xp = -1.5 if i % 2 else 1.5
        js.append(
            f'      tl.fromTo("#p{i:02d}-img", {{ scale: {sx}, xPercent: 0 }}, '
            f'{{ scale: {ex}, xPercent: {xp}, duration: {f(d)}, ease: "none", immediateRender: false }}, {f(s)});'
        )
    return "\n".join(js)


# ============================================================== YOUTUBE 16:9
def build_youtube():
    W, H = 1920, 1080
    total = YT_BARS * BAR
    clips, photos, sfx_list = [], [], []
    vi = pi = si = 0
    cut_times = []
    for kind, src, sb, eb, ms in YT_SHOTS:
        s, e = sb * BAR, eb * BAR
        cut_times.append(s)
        if kind == "v":
            vi += 1
            clips.append(video_clip(vi, src, s, e, ms))
        else:
            pi += 1
            clips.append(photo_clip(pi, src, s, e))
            photos.append((pi, (s, e)))

    # SFX: kick on every cut, clap on chapter-card starts, chime on the end card.
    card_starts = {c[1] * BAR for c in YT_CARDS}
    for t in cut_times:
        si += 1
        sfx_list.append(sfx(si, "clap" if t in card_starts else "kick", t, 0.55 if t in card_starts else 0.35))
    si += 1
    sfx_list.append(sfx(si, "stab", BAR * 1, 0.6))
    si += 1
    sfx_list.append(sfx(si, "pluck", BAR * 2, 0.6))
    si += 1
    sfx_list.append(sfx(si, "riser", BAR * 74, 0.5))
    si += 1
    sfx_list.append(sfx(si, "chime", BAR * 76, 0.9))

    cards_html = []
    for cid, sb, num, title, line in YT_CARDS:
        s = sb * BAR
        cards_html.append(
            f'<section id="{cid}" class="clip card" data-start="{f(s)}" data-duration="{f(BAR * 1.5)}" '
            f'data-track-index="3" style="z-index:5">'
            f'<div id="{cid}-panel" class="panel"></div>'
            f'<div id="{cid}-box" class="cardbox">'
            f'<div id="{cid}-num" class="mono cardnum">{num}</div>'
            f'<h2 id="{cid}-title" class="display cardtitle">{title}</h2>'
            f'<p id="{cid}-line" class="mono cardline">{line}</p></div></section>'
        )
    # persistent act labels (bottom-right)
    labels_html = []
    bounds = [c[1] for c in YT_CARDS] + [76]
    for k, (cid, sb, num, title, line) in enumerate(YT_CARDS):
        s = sb * BAR + BAR * 1.5
        e = bounds[k + 1] * BAR
        labels_html.append(
            f'<div id="l{num}" class="clip actlabel mono" data-start="{f(s)}" data-duration="{f(e - s)}" '
            f'data-track-index="5" style="z-index:6"><span id="l{num}-t">{num} — {title}</span></div>'
        )

    open_html = f"""
      <section id="open" class="clip overlay" data-start="0" data-duration="{f(BAR * 4)}" data-track-index="3" style="z-index:5">
        <div id="open-shade" class="shade"></div>
        <div id="open-box" class="openbox">
          <h1 id="open-title" class="display opentitle"><span id="ot1" class="w">DJ</span> <span id="ot2" class="w">DOORBELL</span></h1>
          <p id="open-sub" class="mono opensub">a doorbell you <em>play</em>, not press.</p>
        </div>
      </section>"""
    end_html = f"""
      <section id="end" class="clip overlay endcard" data-start="{f(BAR * 76)}" data-duration="{f(total - BAR * 76)}" data-track-index="3" style="z-index:5">
        <div id="end-box" class="endbox">
          <div id="end-kicker" class="mono endkicker">TO BE CONTINUED</div>
          <h1 id="end-title" class="display endtitle">PART 2:<br>INSTALL DAY</h1>
          <p id="end-url" class="mono endurl">github.com/andzhelo/DjDoorbell</p>
        </div>
        <div id="end-fade" class="fade"></div>
      </section>"""

    cards_js = []
    for cid, sb, num, title, line in YT_CARDS:
        s = sb * BAR
        d = BAR * 1.5
        cards_js.append(f"""
      tl.fromTo("#{cid}-panel", {{ clipPath: "inset(0 100% 0 0)" }}, {{ clipPath: "inset(0 0% 0 0)", duration: 0.28, ease: "power4.out", immediateRender: false }}, {f(s)});
      tl.fromTo("#{cid}-num", {{ opacity: 0, x: -60 }}, {{ opacity: 1, x: 0, duration: 0.35, ease: "expo.out", immediateRender: false }}, {f(s + 0.12)});
      tl.fromTo("#{cid}-title", {{ opacity: 0, y: 70, scale: 1.08 }}, {{ opacity: 1, y: 0, scale: 1, duration: 0.45, ease: "power4.out", immediateRender: false }}, {f(s + 0.20)});
      tl.fromTo("#{cid}-line", {{ opacity: 0, y: 30 }}, {{ opacity: 1, y: 0, duration: 0.4, ease: "power3.out", immediateRender: false }}, {f(s + 0.45)});
      tl.to("#{cid}-panel", {{ clipPath: "inset(0 0 0 100%)", duration: 0.32, ease: "power4.in" }}, {f(s + d - 0.34)});
      tl.to("#{cid}-box", {{ x: 260, opacity: 0, duration: 0.3, ease: "power4.in" }}, {f(s + d - 0.32)});""")

    labels_js = []
    for cid, sb, num, title, line in YT_CARDS:
        s = sb * BAR + BAR * 1.5
        labels_js.append(
            f'      tl.fromTo("#l{num}-t", {{ opacity: 0, x: 40 }}, {{ opacity: 1, x: 0, duration: 0.4, ease: "power3.out", immediateRender: false }}, {f(s)});'
        )

    html = f"""<!doctype html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width={W}, height={H}" />
    <title>DJ Doorbell — build, part 1</title>
    <script src="https://cdn.jsdelivr.net/npm/gsap@3.14.2/dist/gsap.min.js"></script>
    <style>
      * {{ margin: 0; padding: 0; box-sizing: border-box; }}
      html, body {{ width: {W}px; height: {H}px; overflow: hidden; background: {BG}; color: {FG}; }}
      #root {{ position: relative; width: 100%; height: 100%; overflow: hidden; background: {BG}; }}
      .clip {{ position: absolute; inset: 0; }}
      .vid {{ width: 100%; height: 100%; object-fit: cover; background: {BG}; }}
      .photo {{ overflow: hidden; background: {BG}; }}
      .photo img {{ position: absolute; inset: 0; width: 100%; height: 100%; display: block; will-change: transform; }}
      .photo img.pbg {{ object-fit: cover; filter: blur(28px) brightness(0.55) saturate(0.8); transform: scale(1.15); }}
      .photo img.kb {{ object-fit: contain; }}
{FONT_CSS}
      /* ---- cold open ---- */
      .shade {{ position: absolute; inset: 0; background: radial-gradient(120% 90% at 20% 80%, rgba(13,15,10,0.92) 0%, rgba(13,15,10,0.55) 45%, rgba(13,15,10,0.05) 100%); }}
      .openbox {{ position: absolute; left: 110px; bottom: 130px; max-width: 1400px; }}
      .opentitle {{ font-size: 220px; line-height: 0.86; letter-spacing: -0.035em; color: {FG}; }}
      .opentitle .w {{ display: inline-block; opacity: 0; }}
      .opentitle #ot2 {{ color: {LIME}; }}
      .opensub {{ margin-top: 34px; font-size: 40px; letter-spacing: 0.02em; color: {FG}; opacity: 0; }}
      .opensub em {{ font-style: normal; color: {LIME}; }}
      /* ---- chapter cards ---- */
      .card {{ overflow: hidden; }}
      .panel {{ position: absolute; inset: 0; background: {BG}; }}
      .cardbox {{ position: absolute; left: 140px; top: 300px; width: 1640px; }}
      .cardnum {{ font-size: 44px; color: {LIME}; letter-spacing: 0.12em; opacity: 0; }}
      .cardtitle {{ margin-top: 18px; font-size: 168px; line-height: 0.9; letter-spacing: -0.03em; color: {FG}; opacity: 0; }}
      .cardline {{ margin-top: 40px; font-size: 38px; color: {MUTED}; letter-spacing: 0.01em; opacity: 0; }}
      .actlabel {{ inset: auto 80px 72px auto; font-size: 24px; letter-spacing: 0.14em; color: {FG}; text-shadow: 0 2px 12px rgba(0,0,0,0.8); }}
      .actlabel span {{ display: inline-block; opacity: 0; padding: 10px 18px; border: 3px solid {LIME}; background: rgba(13,15,10,0.55); }}
      /* ---- end card ---- */
      .endcard {{ background: {BG}; }}
      .endbox {{ position: absolute; left: 140px; top: 260px; width: 1640px; }}
      .endkicker {{ font-size: 40px; letter-spacing: 0.2em; color: {LIME}; opacity: 0; }}
      .endtitle {{ margin-top: 24px; font-size: 200px; line-height: 0.9; letter-spacing: -0.035em; color: {FG}; opacity: 0; }}
      .endurl {{ margin-top: 48px; font-size: 44px; color: {MUTED}; opacity: 0; }}
      .fade {{ position: absolute; inset: 0; background: #000; opacity: 0; }}
      /* ---- HUD: the pad grid + 16-step bar ---- */
      .hud {{ pointer-events: none; }}
      .padgrid {{ position: absolute; left: 80px; bottom: 150px; display: grid; grid-template-columns: repeat(3, 26px); gap: 12px; }}
      .pad {{ display: block; width: 26px; height: 26px; border-radius: 50%; opacity: 0.35; will-change: transform; box-shadow: 0 0 18px 2px currentColor; }}
      .pad.r1 {{ background: {AMBER}; color: {AMBER}; }}
      .pad.r2 {{ background: {CYAN}; color: {CYAN}; }}
      .pad.r3 {{ background: {MAGENTA}; color: {MAGENTA}; }}
      .steps {{ position: absolute; left: 80px; bottom: 112px; display: flex; gap: 6px; }}
      .step {{ display: block; width: 12px; height: 14px; background: {LIME}; opacity: 0.25; transform-origin: bottom; will-change: transform; }}
      .hudlabel {{ position: absolute; left: 80px; bottom: 72px; font-size: 22px; letter-spacing: 0.16em; color: {LIME}; background: rgba(13,15,10,0.84); padding: 6px 12px; }}
    </style>
  </head>
  <body>
    <div id="root" data-composition-id="main" data-start="0" data-width="{W}" data-height="{H}" data-duration="{f(total)}" data-fps="30">
      {chr(10).join("      " + c for c in clips)}
{open_html}
      {chr(10).join("      " + c for c in cards_html)}
      {chr(10).join("      " + c for c in labels_html)}
{end_html}
{hud(total)}
      {chr(10).join("      " + a for a in bed(total))}
      {chr(10).join("      " + a for a in sfx_list)}
    </div>
    <script>
      const tl = gsap.timeline({{ paused: true }});
{HUD_JS % dict(beat=BEAT, bar=BAR, total=total)}
      // ---- cold open: title slams on bar 1, subtitle on bar 2 ----
      tl.fromTo("#ot1", {{ opacity: 0, scale: 1.6, filter: "blur(18px)" }}, {{ opacity: 1, scale: 1, filter: "blur(0px)", duration: 0.45, ease: "power4.out", immediateRender: false }}, BAR * 1);
      tl.fromTo("#ot2", {{ opacity: 0, x: -260 }}, {{ opacity: 1, x: 0, duration: 0.42, ease: "expo.out", immediateRender: false }}, BAR * 1 + BEAT);
      tl.fromTo("#open-sub", {{ opacity: 0, y: 40 }}, {{ opacity: 1, y: 0, duration: 0.5, ease: "circ.out", immediateRender: false }}, BAR * 2);
      // ---- photos: Ken Burns on an inner img (never on the timed clip) ----
{ken_burns_js(photos)}
      // ---- chapter cards: wipe in, slam, wipe out (the wipe IS the exit) ----
{chr(10).join(cards_js)}
{chr(10).join(labels_js)}
      // ---- end card (final scene: exit fades allowed) ----
      const E = BAR * 76;
      tl.fromTo("#end-kicker", {{ opacity: 0, x: -50 }}, {{ opacity: 1, x: 0, duration: 0.4, ease: "expo.out", immediateRender: false }}, E + 0.1);
      tl.fromTo("#end-title", {{ opacity: 0, y: 80 }}, {{ opacity: 1, y: 0, duration: 0.55, ease: "power4.out", immediateRender: false }}, E + BEAT);
      tl.fromTo("#end-url", {{ opacity: 0, y: 30 }}, {{ opacity: 1, y: 0, duration: 0.5, ease: "power3.out", immediateRender: false }}, E + BAR);
      tl.to("#end-fade", {{ opacity: 1, duration: 1.2, ease: "power2.in" }}, TOTAL - 1.3);
      window.__timelines["main"] = tl;
      tl.seek(0);
    </script>
  </body>
</html>
"""
    (HERE / "index.html").write_text(html)
    return total


# ================================================================ REEL 9:16
def build_reel():
    W, H = 1080, 1920
    total = REEL_BARS * BAR
    clips, caps_html, caps_js, sfx_list = [], [], [], []
    vi = si = 0
    for k, (kind, src, sb, eb, ms, cap) in enumerate(REEL_SHOTS):
        s, e = sb * BAR, eb * BAR
        vi += 1
        cls = "tall" if src in VERTICAL else "cover"
        clips.append(video_clip(vi, src, s, e, ms, extra_class=cls))
        si += 1
        sfx_list.append(sfx(si, "kick", s, 0.4))
        if cap:
            cid = f"cap{k:02d}"
            caps_html.append(
                f'<section id="{cid}" class="clip capwrap" data-start="{f(s)}" data-duration="{f(e - s)}" '
                f'data-track-index="3" style="z-index:5"><h2 id="{cid}-t" class="display cap"><span>{cap}</span></h2>'
                f'<i id="{cid}-bar" class="capbar"></i></section>'
            )
            caps_js.append(
                f'      tl.fromTo("#{cid}-bar", {{ scaleX: 0 }}, {{ scaleX: 1, duration: 0.3, ease: "power4.out", immediateRender: false }}, {f(s + BEAT * 0.5)});\n'
                f'      tl.fromTo("#{cid}-t", {{ opacity: 0, y: 60, skewY: 4 }}, {{ opacity: 1, y: 0, skewY: 0, duration: 0.4, ease: "power4.out", immediateRender: false }}, {f(s + BEAT * 0.5)});'
            )
    si += 1
    sfx_list.append(sfx(si, "clap", BEAT * 2, 0.6))
    si += 1
    sfx_list.append(sfx(si, "stab", BAR * 1, 0.6))
    si += 1
    sfx_list.append(sfx(si, "riser", BAR * 16, 0.6))
    si += 1
    sfx_list.append(sfx(si, "chime", BAR * 18, 0.9))

    open_html = f"""
      <section id="open" class="clip overlay" data-start="0" data-duration="{f(BAR * 2)}" data-track-index="3" style="z-index:5">
        <div id="open-shade" class="shade"></div>
        <div id="open-box" class="openbox">
          <div id="ot0" class="mono kicker">I BUILT A DOORBELL</div>
          <h1 id="open-title" class="display opentitle"><span id="ot1" class="w">YOU</span><br><span id="ot2" class="w lime">PLAY.</span></h1>
        </div>
      </section>"""
    end_html = f"""
      <section id="end" class="clip overlay" data-start="{f(BAR * 18)}" data-duration="{f(BAR * 2)}" data-track-index="3" style="z-index:7">
        <div id="end-url" class="mono endurl">github.com/andzhelo/DjDoorbell</div>
        <div id="end-fade" class="fade"></div>
      </section>"""

    html = f"""<!doctype html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width={W}, height={H}" />
    <title>DJ Doorbell — reel</title>
    <script src="https://cdn.jsdelivr.net/npm/gsap@3.14.2/dist/gsap.min.js"></script>
    <style>
      * {{ margin: 0; padding: 0; box-sizing: border-box; }}
      html, body {{ width: {W}px; height: {H}px; overflow: hidden; background: {BG}; color: {FG}; }}
      #root {{ position: relative; width: 100%; height: 100%; overflow: hidden; background: {BG}; }}
      .clip {{ position: absolute; inset: 0; }}
      .vid {{ width: 100%; height: 100%; object-fit: cover; object-position: 50% 45%; background: {BG}; }}
{FONT_CSS}
      .shade {{ position: absolute; inset: 0; background: radial-gradient(90% 60% at 50% 85%, rgba(13,15,10,0.92) 0%, rgba(13,15,10,0.5) 50%, rgba(13,15,10,0.05) 100%); }}
      .openbox {{ position: absolute; left: 70px; right: 70px; bottom: 560px; }}
      .kicker {{ font-size: 44px; letter-spacing: 0.18em; color: {FG}; opacity: 0; }}
      .opentitle {{ margin-top: 18px; font-size: 236px; line-height: 0.86; letter-spacing: -0.035em; color: {FG}; }}
      .opentitle .w {{ display: inline-block; opacity: 0; }}
      .lime {{ color: {LIME}; }}
      .capwrap {{ pointer-events: none; }}
      .cap {{ position: absolute; left: 70px; right: 70px; bottom: 640px; font-size: 104px; line-height: 1.02; letter-spacing: -0.03em; color: {FG}; opacity: 0; }}
      .cap span {{ display: inline; background: rgba(13,15,10,0.84); padding: 6px 22px; -webkit-box-decoration-break: clone; box-decoration-break: clone; }}
      .capbar {{ position: absolute; left: 70px; bottom: 600px; width: 220px; height: 14px; background: {LIME}; transform-origin: left; display: block; }}
      .endurl {{ position: absolute; left: 70px; bottom: 520px; font-size: 40px; letter-spacing: 0.04em; color: {LIME}; background: rgba(13,15,10,0.84); padding: 10px 18px; opacity: 0; }}
      .fade {{ position: absolute; inset: 0; background: #000; opacity: 0; }}
      .hud {{ pointer-events: none; }}
      .padgrid {{ position: absolute; left: 70px; top: 190px; display: grid; grid-template-columns: repeat(3, 30px); gap: 14px; }}
      .pad {{ display: block; width: 30px; height: 30px; border-radius: 50%; opacity: 0.35; will-change: transform; box-shadow: 0 0 20px 2px currentColor; }}
      .pad.r1 {{ background: {AMBER}; color: {AMBER}; }}
      .pad.r2 {{ background: {CYAN}; color: {CYAN}; }}
      .pad.r3 {{ background: {MAGENTA}; color: {MAGENTA}; }}
      .steps {{ position: absolute; left: 70px; top: 330px; display: flex; gap: 6px; }}
      .step {{ display: block; width: 12px; height: 16px; background: {LIME}; opacity: 0.25; transform-origin: bottom; will-change: transform; }}
      .hudlabel {{ position: absolute; left: 70px; top: 364px; font-size: 24px; letter-spacing: 0.16em; color: {LIME}; background: rgba(13,15,10,0.84); padding: 6px 12px; }}
    </style>
  </head>
  <body>
    <div id="root" data-composition-id="main" data-start="0" data-width="{W}" data-height="{H}" data-duration="{f(total)}" data-fps="30">
      {chr(10).join("      " + c for c in clips)}
{open_html}
      {chr(10).join("      " + c for c in caps_html)}
{end_html}
{hud(total)}
      {chr(10).join("      " + a for a in bed(total, vol=0.5, fade=1.2))}
      {chr(10).join("      " + a for a in sfx_list)}
    </div>
    <script>
      const tl = gsap.timeline({{ paused: true }});
{HUD_JS % dict(beat=BEAT, bar=BAR, total=total)}
      tl.fromTo("#ot0", {{ opacity: 0, y: 30 }}, {{ opacity: 1, y: 0, duration: 0.35, ease: "power3.out", immediateRender: false }}, BEAT * 1);
      tl.fromTo("#ot1", {{ opacity: 0, scale: 1.6, filter: "blur(18px)" }}, {{ opacity: 1, scale: 1, filter: "blur(0px)", duration: 0.42, ease: "power4.out", immediateRender: false }}, BEAT * 2);
      tl.fromTo("#ot2", {{ opacity: 0, x: -220 }}, {{ opacity: 1, x: 0, duration: 0.4, ease: "expo.out", immediateRender: false }}, BEAT * 3);
{chr(10).join(caps_js)}
      tl.fromTo("#end-url", {{ opacity: 0, y: 30 }}, {{ opacity: 1, y: 0, duration: 0.4, ease: "power3.out", immediateRender: false }}, BAR * 18 + BEAT * 2);
      tl.to("#end-fade", {{ opacity: 1, duration: 0.8, ease: "power2.in" }}, TOTAL - 0.85);
      window.__timelines["main"] = tl;
      tl.seek(0);
    </script>
  </body>
</html>
"""
    (HERE.parent / "djdoorbell-reel" / "index.html").write_text(html)
    return total


if __name__ == "__main__":
    yt = build_youtube()
    reel = build_reel()
    print(f"index.html  {yt:.2f} s ({int(yt//60)}:{yt%60:04.1f})")
    print(f"../djdoorbell-reel/index.html   {reel:.2f} s")
