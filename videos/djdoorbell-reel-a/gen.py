#!/usr/bin/env python3
"""Writes index.html and compositions/*.html for the DJ Doorbell reel (storyline A).
All timing is in bars of the doorbell's own track: 124 BPM, bar = 1.93548 s.
Run: python3 gen.py  (then npx hyperframes check)."""
from pathlib import Path

HERE = Path(__file__).parent
S16 = 60 / 124 / 4
BAR = 16 * S16            # 1.93548
BEAT = 4 * S16
BARS = 19
TOTAL = 19 * (16 * 60 / 124 / 4)   # exactly 19 bars so the loop point is a bar line

LIME = "#c8ff2e"
HOOK_T = 8.03   # door_7119 time at frame 0: just after a sound tail that ends at 8.02 s (v3 started at 7.9 and cut into it)
W, H = 1080, 1920

def f(x): return f"{x:.4f}".rstrip("0").rstrip(".")

# ---------------------------------------------------------------- helpers
def words(text):
    """Title text -> spans, *word* marks the accent word(s)."""
    out = []
    for w in text.split(" "):
        acc = w.startswith("*") and w.endswith("*")
        w = w.strip("*")
        out.append(f'<span class="w{" acc" if acc else ""}">{w}</span>')
    return " ".join(out)

def title(id_, lines, start, dur, pos="low", size=120, instant=False):
    """A title of 1-3 lines. pos: 'low' (y 1000-1300) or 'top' (y 230-650)."""
    html = (f'<div id="{id_}" class="clip title {pos}" data-start="{f(start)}" data-duration="{f(dur)}" '
            f'data-track-index="5" style="font-size:{size}px">'
            + "".join(f'<div class="ln">{words(l)}</div>' for l in lines) + "</div>")
    if instant:
        js = ""
    else:
        js = (f'tl.fromTo("#{id_} .w", {{opacity: 0, filter: "blur(6px)"}}, '
              f'{{opacity: 1, filter: "blur(0px)", duration: 0.13, stagger: 0.067, ease: "power3.out"}}, {f(start)});\n')
    return html, js

def video(id_, src, start, dur, media_start=0, pos="50% 50%", z=1, rate=None, sound=None,
          fade_in=None, fade_out=None, extra=""):
    """sound=None mutes the clip; sound=<gain> keeps the clip's own audio at that gain.
    fade_in/fade_out: clip-edge audio fades in seconds. extra: more attributes (automation, fx)."""
    r = f' data-playback-rate="{rate}"' if rate else ""
    a = f' data-has-audio="true" data-volume="{sound}"' if sound else " muted"
    if sound and fade_in: a += f' data-fade-in="{fade_in}"'
    if sound and fade_out: a += f' data-fade-out="{fade_out}"'
    if sound and extra: a += " " + extra
    return (f'<video id="{id_}" class="clip media"{a} playsinline preload="auto" src="assets/video/{src}.mp4" '
            f'data-start="{f(start)}" data-duration="{f(dur)}" data-media-start="{f(media_start)}"{r} '
            f'data-track-index="1" style="object-position:{pos};z-index:{z}"></video>')

def photo(id_, src, start, dur, pos="50% 50%", push=1.08, z=1):
    """Still with a slow push: a div with the photo as background (not an <img>, which the
    runtime lays out as its own media clip), GSAP scales the div."""
    html = (f'<div id="{id_}" class="clip media photo" data-start="{f(start)}" data-duration="{f(dur)}" data-track-index="1" '
            f'style="background-image:url(assets/photo/{src}.jpg);background-position:{pos};z-index:{z}"></div>')
    js = f'tl.fromTo("#{id_}", {{scale: 1}}, {{scale: {push}, duration: {f(dur)}, ease: "none"}}, {f(start)});\n'
    return html, js

SCENE_CSS = """
    #root { position: absolute; inset: 0; background: #000; overflow: hidden; }
    .media { position: absolute; left: 0; top: 0; width: 1080px; height: 1920px; object-fit: cover; }
    .media { transform-origin: 50% 50%; }
    .photo { background-size: cover; background-repeat: no-repeat; }
    .shade { position: absolute; left: 0; right: 0; bottom: 0; height: 900px; z-index: 40 !important;
             background: linear-gradient(180deg, rgba(0,0,0,0) 0%, rgba(0,0,0,0.72) 70%, rgba(0,0,0,0.8) 100%); }
    .shade.top { top: 0; bottom: auto; height: 820px;
             background: linear-gradient(0deg, rgba(0,0,0,0) 0%, rgba(0,0,0,0.68) 60%, rgba(0,0,0,0.78) 100%); }
    .title { position: absolute; left: 72px; right: 72px; z-index: 60 !important; font-family: "League Gothic", "Oswald", sans-serif;
             font-weight: 400; text-transform: uppercase; color: #fff; line-height: 0.92; letter-spacing: 0.01em;
             text-shadow: 0 2px 10px rgba(0,0,0,0.6), 0 4px 40px rgba(0,0,0,0.5); }
    .title.low { bottom: 620px; }
    .title.top { top: 230px; }
    .title .ln { display: block; }
    .title .w { display: inline-block; will-change: filter, opacity; }
    .title .acc { color: __LIME__; }
    .small { position: absolute; left: 72px; right: 72px; bottom: 560px; z-index: 60 !important; font-family: "Inter", sans-serif;
             font-weight: 700; font-size: 44px; color: #fff; letter-spacing: 0.02em; }
""".replace("__LIME__", LIME)

def scene(id_, dur, body, js, extra_css=""):
    return f"""<template>
  <style>{SCENE_CSS}{extra_css}
  </style>
  <div id="root" data-composition-id="{id_}" data-width="{W}" data-height="{H}">
{body}
  </div>
  <script>
    const tl = gsap.timeline({{ paused: true }});
{js}    window.__timelines["{id_}"] = tl;
  </script>
</template>
"""

# ---------------------------------------------------------------- scenes
scenes = []   # (id, start_bar, bars, html)

def add(id_, start_bar, bars, parts, extra_css=""):
    body, js = [], []
    for p in parts:
        if isinstance(p, tuple):
            body.append(p[0]); js.append(p[1])
        else:
            body.append(p)
    scenes.append((id_, start_bar, bars, scene(id_, bars * BAR, "\n".join(body), "".join(js), extra_css)))

# b0-1 HOOK: the hand on the pad, title already on frame 0
HOOK_LANE = ('{"version":1,"lanes":[{"target":"volume","points":[{"t":0,"v":0.615},{"t":%s,"v":0.615},{"t":%s,"v":1}]}]}'
             % (f(0.9 * BAR), f(1.4 * BAR)))
HOOK_FX = '{"version":1,"nodes":[{"type":"highpass","id":"hp","label":"Remove rumble","params":{"frequency":90}}]}'
HOOK_AUDIO = ("data-automation='%s' data-fx-chain=\"%s\"" % (HOOK_LANE, HOOK_FX.replace('"', "&quot;")))
hook_t = title("hook-t", ["my doorbell", "is a", "*drum* *machine*"], 0, 2 * BAR, pos="top", size=168, instant=True)
add("hook", 0, 2, [
    # the real door sound. data-volume 2.6 x lane: 1.6 in bar 0 (dense playing), rising to 2.6 through
    # bar 1 where the clip's own presses are ~7 dB quieter, so the hand-off to the generated track is level.
    video("hook-v", "door_7119", 0, 2 * BAR, media_start=HOOK_T, sound=2.6, fade_in=0.04,
          extra=HOOK_AUDIO),
    '<div class="shade top"></div>',
    hook_t,
])

# b2-3 STARTED: old button, the meter, the chime box
add("started", 2, 2, [
    video("st-v1", "wall_6925", 0, BAR, media_start=1.0, pos="22% 50%"),
    photo("st-p1", "p6338", BAR, BAR / 2, pos="50% 40%", push=1.10),
    photo("st-p2", "p6339", 1.5 * BAR, BAR / 2, pos="50% 50%", push=1.10),
    '<div class="shade"></div>',
    title("st-t1", ["it started", "with *this*"], 0, BAR),
    title("st-t2", ["one button.", "*8* *volts.*"], BAR, BAR),
])

# b4-5 ORDERED: parts on the beat
add("ordered", 4, 2, [
    photo("or-p1", "p6807", 0, 2 * BEAT, pos="50% 50%", push=1.06),
    photo("or-p2", "p7017", 2 * BEAT, BEAT, pos="50% 45%", push=1.08),
    photo("or-p3", "p7018", 3 * BEAT, BEAT, pos="50% 50%", push=1.08),
    photo("or-p4", "p6819", 4 * BEAT, BEAT, pos="50% 50%", push=1.08),
    photo("or-p5", "p6703", 5 * BEAT, BEAT, pos="50% 50%", push=1.08),
    photo("or-p6", "p6704", 6 * BEAT, 2 * BEAT, pos="50% 50%", push=1.06),
    '<div class="shade"></div>',
    title("or-t1", ["so I ordered", "*everything*"], 0, 2 * BAR),
])

# b6-10 BUILD: one shot and one line per bar
add("build", 6, 5, [
    photo("bu-p1", "p6770", 0, BAR, pos="50% 50%", push=1.10),
    video("bu-v2", "face_6898", BAR, BAR, media_start=1.0, pos="62% 40%"),
    video("bu-v3", "solder_6899", 2 * BAR, BAR, media_start=1.5, pos="50% 50%"),
    video("bu-v4", "tl_6912", 3 * BAR, BAR, media_start=6.0),
    video("bu-v5", "strip_vid", 4 * BAR, BAR, media_start=1.4),
    '<div class="shade"></div>',
    title("bu-t1", ["*24* mm hole.", "2.5 mm plate."], 0, BAR),
    title("bu-t2", ["*nine* arcade", "buttons"], BAR, BAR),
    title("bu-t3", ["nine wires.", "*one* ground."], 2 * BAR, BAR),
    title("bu-t4", ["*24* minutes", "of soldering"], 3 * BAR, BAR),
    title("bu-t5", ["*first* light"], 4 * BAR, BAR, size=150),
])

# b11 FIVE BOARDS
add("boards", 11, 1, [
    photo("bo-p1", "p7017", 0, BAR / 2, pos="50% 45%", push=1.12),
    photo("bo-p2", "p7121", BAR / 2, BAR / 2, pos="50% 50%", push=1.08),
    '<div class="shade"></div>',
    title("bo-t1", ["*5* boards.", "1 survived."], 0, BAR, size=150),
], extra_css=f"\n    #bo-t1 .acc {{ color: #ff2bd6; }}")

# b12-13 INSTALL
add("install", 12, 2, [
    photo("in-p1", "p7107", 0, BAR / 2, pos="50% 50%", push=1.08),
    photo("in-p2", "p7111", BAR / 2, BAR / 2, pos="50% 55%", push=1.08),
    video("in-v3", "shellwall_6927", BAR, BAR, media_start=1.0, pos="68% 50%"),
    '<div class="shade"></div>',
    title("in-t1", ["breaker off.", "*12* V on the pair."], 0, BAR),
    title("in-t2", ["on the *wall*"], BAR, BAR, size=150),
])

# b14-15 PAYOFF: the door, full bleed, the claim on the second bar
add("payoff", 14, 2, [
    video("pa-v1", "door_7119", 0, 2 * BAR, media_start=9.6),
    '<div id="pa-shade" class="clip shade top" data-start="%s" data-duration="%s" data-track-index="4"></div>' % (f(BAR), f(BAR)),
    title("pa-t1", ["every visitor", "is a *DJ*"], BAR, BAR, pos="top", size=168),
])

# b16 PHONE
add("phone", 16, 1, [
    video("ph-v1", "phone_7123", 0, BAR, media_start=4.0),
    '<div class="shade top"></div>',
    title("ph-t1", ["or from", "the *phone*"], 0, BAR, pos="top", size=150),
])

# b17 END CARD: lime field wipes up, name and link
TAIL_AUDIO = 'data-fx-chain="%s"' % HOOK_FX.replace('"', "&quot;")
end_css = f"""
    #end-field {{ position: absolute; inset: 0; background: {LIME}; z-index: 20 !important; transform: translateY(1920px); }}
    #end-name {{ position: absolute; left: 72px; right: 72px; top: 700px; z-index: 30 !important; font-family: "League Gothic", sans-serif;
                 font-size: 256px; line-height: 0.9; text-transform: uppercase; color: #0f110b; }}
    #end-link {{ position: absolute; left: 72px; right: 72px; top: 1290px; z-index: 30 !important; font-family: "Inter", sans-serif;
                 font-weight: 700; font-size: 44px; color: #0f110b; letter-spacing: 0.01em; }}
    #end-sub {{ position: absolute; left: 72px; right: 72px; top: 560px; z-index: 30 !important; font-family: "Inter", sans-serif;
                 font-weight: 700; font-size: 40px; color: #0f110b; letter-spacing: 0.12em; text-transform: uppercase; }}
"""
end_js = (
    'tl.fromTo("#end-field", {y: 1920}, {y: 0, duration: 0.4, ease: "power3.inOut"}, 0);\n'
    'tl.fromTo("#end-name .w", {opacity: 0, y: 40}, {opacity: 1, y: 0, duration: 0.3, stagger: 0.08, ease: "power4.out"}, 0.3);\n'
    'tl.fromTo("#end-sub", {opacity: 0}, {opacity: 1, duration: 0.2, ease: "power2.out"}, 0.55);\n'
    'tl.fromTo("#end-link", {opacity: 0}, {opacity: 1, duration: 0.2, ease: "power2.out"}, 0.7);\n'
)
add("end", 17, 2, [
    video("end-v", "door_7119", 0, 1.5 * BAR, media_start=13.5, z=1),
    f'<div id="end-field" class="clip" data-start="0" data-duration="{f(1.5 * BAR)}" data-track-index="3" data-layout-allow-overflow></div>',
    f'<div id="end-sub" class="clip" data-start="0" data-duration="{f(1.5 * BAR)}" data-track-index="5">open source · esp32 · 3d printed</div>',
    f'<div id="end-name" class="clip" data-start="0" data-duration="{f(1.5 * BAR)}" data-track-index="5"><div class="ln"><span class="w">DJ</span></div><div class="ln"><span class="w">Doorbell</span></div></div>',
    f'<div id="end-link" class="clip" data-start="0" data-duration="{f(1.5 * BAR)}" data-track-index="5">github.com/andzhelo/DjDoorbell</div>',
    (" ", end_js),
    # loop tail: the half bar of door footage that leads into frame 0 (7.9 s), same title, real sound
    video("end-tail", "door_7119", 1.5 * BAR, 0.5 * BAR, media_start=HOOK_T - 0.5 * BAR, z=1, sound=1.6,
          fade_in=0.2, fade_out=0.15, extra=TAIL_AUDIO),
    '<div id="end-tail-shade" class="clip shade top" data-start="%s" data-duration="%s" data-track-index="4"></div>' % (f(1.5 * BAR), f(0.5 * BAR)),
    title("end-tail-t", ["my doorbell", "is a", "*drum* *machine*"], 1.5 * BAR, 0.5 * BAR, pos="top", size=168, instant=True),
], extra_css=end_css + '\n    #end-field { z-index: 20 !important; }\n    #end-tail { z-index: 50 !important; }\n    #end-tail-shade { z-index: 55 !important; }\n')

# ---------------------------------------------------------------- write
comp = HERE / "compositions"
comp.mkdir(exist_ok=True)
hosts = []
for id_, sb, bars, html in scenes:
    (comp / f"{id_}.html").write_text(html)
    hosts.append(f'    <div id="{id_}" class="clip" data-composition-src="compositions/{id_}.html" data-composition-id="{id_}" '
                 f'data-start="{f(sb * BAR)}" data-duration="{f(bars * BAR)}" data-track-kind="graphics" data-track-index="0"></div>')

index = f"""<!doctype html>
<html lang="en" data-resolution="portrait" data-composition-variables='[
  {{"id":"accent","type":"color","label":"Accent","default":"{LIME}"}}
]'>
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width={W}, height={H}" />
    <script src="https://cdn.jsdelivr.net/npm/gsap@3.15.0/dist/gsap.min.js"></script>
    <style>
      :root {{ --ground: #000; --accent: {LIME}; }}
      * {{ margin: 0; padding: 0; box-sizing: border-box; }}
      html, body {{ width: {W}px; height: {H}px; overflow: hidden; background: var(--ground); }}
      #root {{ position: relative; width: {W}px; height: {H}px; background: var(--ground); }}
      #root > .clip {{ position: absolute; inset: 0; }}
    </style>
  </head>
  <body>
    <div id="root" data-composition-id="main" data-start="0" data-duration="{f(TOTAL)}" data-width="{W}" data-height="{H}">
{chr(10).join(hosts)}
    <audio id="music" src="assets/audio/track.wav" data-start="{f(2 * BAR)}" data-media-start="{f(2 * BAR)}" data-duration="{f(15.5 * BAR)}" data-volume="1" data-fade-in="0.25" data-fade-out="1.6" data-track-index="8"></audio>
    <audio id="chime" src="assets/audio/chime.wav" data-start="{f(17 * BAR + BEAT)}" data-volume="0.55" data-track-index="9"></audio>
    </div>
    <script>
      const tl = gsap.timeline({{ paused: true }});
      window.__timelines["main"] = tl;
      tl.seek(0);
    </script>
  </body>
</html>
"""
(HERE / "index.html").write_text(index)
print("bar %.5f s, total %.2f s, %d scenes" % (BAR, TOTAL, len(scenes)))
for id_, sb, bars, _ in scenes:
    print(f"  {id_:8s} b{sb:<2d} {bars} bar(s)  {sb*BAR:6.2f}-{(sb+bars)*BAR:6.2f} s  frames {round(sb*BAR*30)}-{round((sb+bars)*BAR*30)}")
