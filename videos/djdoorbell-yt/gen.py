#!/usr/bin/env python3
"""Writes index.html and compositions/*.html for the DJ Doorbell YouTube film (storyline A).
Timing is in bars of the doorbell's own engine: 16 x 2667 samples at 22050 Hz = 1.935238 s.
Run: python3 gen.py && npx hyperframes check"""
from pathlib import Path

HERE = Path(__file__).parent
S16 = 2667 / 22050
BAR = 16 * S16              # 1.935238
BEAT = 4 * S16
BARS = 44
TOTAL = BARS * BAR          # 85.15 s

W, H = 1920, 1080
LIME = "#c8ff2e"
LIME_INK = "#7fb800"        # lines and cells on the paper ground
LIME_TEXT = "#5a8a00"       # the accent word on paper (AA contrast)
PAPER = "#f6f7f1"
INK = "#15170f"
PW = 608                    # pillar width for 9:16 clips at full height
PX = (W - PW) // 2

def f(x): return f"{x:.4f}".rstrip("0").rstrip(".")

# ---------------------------------------------------------------- helpers
def words(text):
    out = []
    for w in text.split(" "):
        acc = w.startswith("*") and w.endswith("*")
        w = w.strip("*")
        out.append(f'<span class="w{" acc" if acc else ""}">{w}</span>')
    return " ".join(out)

def title(id_, lines, start, dur, pos="low", size=150, instant=False, cls=""):
    html = (f'<div id="{id_}" class="clip title {pos} {cls}" data-start="{f(start)}" data-duration="{f(dur)}" '
            f'data-track-index="5" style="font-size:{size}px">'
            + "".join(f'<div class="ln">{words(l)}</div>' for l in lines) + "</div>")
    js = "" if instant else (
        f'tl.fromTo("#{id_} .w", {{opacity: 0, filter: "blur(6px)"}}, '
        f'{{opacity: 1, filter: "blur(0px)", duration: 0.13, stagger: 0.067, ease: "power3.out"}}, {f(start)});\n')
    return html, js

def nodevideo(id_, src, start, dur, media_start):
    """Timed video inside an UNTIMED node frame (never time a video and its wrapper both)."""
    return (f'<video id="{id_}" class="clip nodemedia" muted playsinline preload="auto" src="assets/video/{src}.mp4" '
            f'data-start="{f(start)}" data-duration="{f(dur)}" data-media-start="{f(media_start)}" data-track-index="1"></video>')

def video(id_, src, start, dur, media_start=0, pos="50% 50%", z=1, sound=None, fade_in=None,
          fade_out=None, extra="", cls="media"):
    a = f' data-has-audio="true" data-volume="{sound}"' if sound else " muted"
    if sound and fade_in: a += f' data-fade-in="{fade_in}"'
    if sound and fade_out: a += f' data-fade-out="{fade_out}"'
    if sound and extra: a += " " + extra
    return (f'<video id="{id_}" class="clip {cls}"{a} playsinline preload="auto" src="assets/video/{src}.mp4" '
            f'data-start="{f(start)}" data-duration="{f(dur)}" data-media-start="{f(media_start)}" '
            f'data-track-index="1" style="object-position:{pos};z-index:{z}"></video>')

def pillar(id_, src, start, dur, media_start=0, sound=None, fade_in=None, fade_out=None, extra="",
           left=PX, z=1):
    """A 9:16 clip standing full height on a blurred, darkened copy of itself (made with ffmpeg)."""
    bg = video(id_ + "-bg", src + "_bg", start, dur, media_start=media_start, z=z, cls="media")
    fg = video(id_, src, start, dur, media_start=media_start, z=z + 1, sound=sound, fade_in=fade_in,
               fade_out=fade_out, extra=extra, cls="pillar")
    return bg + "\n" + fg.replace('class="clip pillar"', f'class="clip pillar" data-left="{left}"').replace(
        f'style="object-position:50% 50%;z-index:{z + 1}"', f'style="left:{left}px;z-index:{z + 1}"')

def photo(id_, src, start, dur, pos="50% 50%", push=1.08, z=1):
    html = (f'<div id="{id_}" class="clip media photo" data-start="{f(start)}" data-duration="{f(dur)}" data-track-index="1" '
            f'style="background-image:url(assets/photo/{src}.jpg);background-position:{pos};z-index:{z}"></div>')
    js = f'tl.fromTo("#{id_}", {{scale: 1}}, {{scale: {push}, duration: {f(dur)}, ease: "none"}}, {f(start)});\n'
    return html, js

SCENE_CSS = """
    #root { position: absolute; inset: 0; background: #000; overflow: hidden; }
    .media { position: absolute; left: 0; top: 0; width: 1920px; height: 1080px; object-fit: cover; transform-origin: 50% 50%; }
    .pillar { position: absolute; top: 0; width: 608px; height: 1080px; object-fit: cover; }
    .photo { background-size: cover; background-repeat: no-repeat; }
    .shade { position: absolute; left: 0; right: 0; bottom: 0; height: 420px; z-index: 40 !important;
             background: linear-gradient(180deg, rgba(0,0,0,0) 0%, rgba(0,0,0,0.62) 60%, rgba(0,0,0,0.72) 100%); }
    .shade.top { top: 0; bottom: auto; height: 440px;
             background: linear-gradient(0deg, rgba(0,0,0,0) 0%, rgba(0,0,0,0.7) 55%, rgba(0,0,0,0.82) 100%); }
    .title { position: absolute; left: 110px; width: 1100px; z-index: 60 !important; font-family: "League Gothic", "Oswald", sans-serif;
             font-weight: 400; text-transform: uppercase; color: #fff; line-height: 0.92; letter-spacing: 0.01em;
             text-shadow: 0 2px 10px rgba(0,0,0,0.6), 0 4px 40px rgba(0,0,0,0.5); }
    .title.low { bottom: 120px; }
    .title.top { top: 110px; }
    .title .ln { display: block; }
    .title .w { display: inline-block; will-change: filter, opacity; }
    .title .acc { color: __LIME__; }
    .title.paper { color: __INK__; text-shadow: none; }
    .title.paper .acc { color: __LIME_TEXT__; }
""".replace("__LIME__", LIME).replace("__INK__", INK).replace("__LIME_TEXT__", LIME_TEXT)

def scene(id_, body, js, extra_css=""):
    return f"""<template>
  <style>{SCENE_CSS}{extra_css}
  </style>
  <div id="root" data-composition-id="{id_}" data-width="{W}" data-height="{H}" data-layout-allow-overlap>
{body}
  </div>
  <script>
    const tl = gsap.timeline({{ paused: true }});
{js}    window.__timelines["{id_}"] = tl;
  </script>
</template>
"""

scenes = []
def add(id_, start_bar, bars, parts, extra_css=""):
    body, js = [], []
    for p in parts:
        if isinstance(p, tuple):
            body.append(p[0]); js.append(p[1])
        else:
            body.append(p)
    scenes.append((id_, start_bar, bars, scene(id_, "\n".join(body), "".join(js), extra_css)))

HP = '{"version":1,"nodes":[{"type":"highpass","id":"hp","label":"Remove rumble","params":{"frequency":90}}]}'
# high-pass (90 Hz) is baked into finished_7157 / door_7119 audio with ffmpeg; no runtime fx chain
HP_ATTR = 'data-fx-chain="%s"' % HP.replace('"', "&quot;")

# ---------------------------------------------------------------- b0-3 HOOK
HOOK_T = 6.0
add("hook", 0, 4, [
    pillar("hk", "finished_7157", 0, 4 * BAR, media_start=HOOK_T, sound=2.2, fade_in=0.05, fade_out=0.1, left=1150),
    '<div class="shade top"></div>',
    title("hk-t", ["my doorbell", "has *no* *button*"], 0, 4 * BAR, pos="top", size=190, instant=True),
])

# ---------------------------------------------------------------- b4-7 THE WALL
add("wall", 4, 4, [
    video("wa-v1", "wall_6925", 0, BAR, media_start=2.0, pos="30% 50%"),
    photo("wa-p1", "p6338", BAR, BAR / 2, pos="50% 40%", push=1.10),
    photo("wa-p2", "p6340", 1.5 * BAR, BAR / 2, pos="50% 50%", push=1.10),
    video("wa-v2", "wires_6926", 2 * BAR, 2 * BAR, media_start=2.5, pos="30% 50%"),
    '<div class="shade"></div>',
    title("wa-t1", ["this is what was", "in the *wall*"], 0, BAR),
    title("wa-t2", ["one button.", "two wires. *8* *volts.*"], BAR, BAR),
    title("wa-t3", ["so the pair", "carries *12* *V*"], 2 * BAR, BAR),
    title("wa-t4", ["and the music", "never leaves the *box*"], 3 * BAR, BAR),
])

# ---------------------------------------------------------------- b8-13 HOW IT WORKS (paper)
NW, NH, GAP = 272, 484, 120
X0 = (W - (5 * NW + 4 * GAP)) // 2
NY = 150
nodes = []   # (id, inner html, label)
grid_cells = "".join(f'<i id="g{i}"></i>' for i in range(16))
nodes.append(("n0", nodevideo("n0-v", "finished_7157", 0, 6 * BAR, 1.5), "the pad"))
nodes.append(("n1", f'<div class="gridbox">{grid_cells}<b>16th grid</b></div>', "the engine"))
nodes.append(("n2", '<div class="radio"><b>30</b><span>bytes<br>per press</span></div>', "radio"))
nodes.append(("n3", nodevideo("n3-v", "finished_7157", 3 * BAR + 0.4, 3 * BAR - 0.4, 15.0), "inside box"))
nodes.append(("n4", nodevideo("n4-v", "phone_7123", 4 * BAR + 0.4, 2 * BAR - 0.4, 4.0), "the phone"))
how_body, how_js = [], []
for k, (nid, inner, label) in enumerate(nodes):
    x = X0 + k * (NW + GAP)
    t = k * BAR
    if k:
        lx = x - GAP
        how_body.append(f'<div id="l{k}" class="clip line" data-start="{f(t)}" data-duration="{f((6 - k) * BAR)}" '
                        f'data-track-index="3" style="left:{lx}px;width:{GAP}px"></div>')
        how_js.append(f'tl.fromTo("#l{k}", {{scaleX: 0}}, {{scaleX: 1, duration: 0.4, ease: "power2.inOut"}}, {f(t)});\n')
    tn = t + (0.4 if k else 0)
    how_body.append(f'<div id="{nid}" class="node" style="left:{x}px">{inner}<em>{label}</em></div>')
    how_js.append(f'tl.fromTo("#{nid}", {{scale: 0.92, opacity: 0}}, {{scale: 1, opacity: 1, duration: 0.27, ease: "power3.out"}}, {f(tn)});\n')
# the door between radio and inside box
dx = X0 + 3 * NW + 2 * GAP + GAP // 2
how_body.append(f'<div id="door" class="clip doormark" data-start="{f(3 * BAR)}" data-duration="{f(3 * BAR)}" data-track-index="3" '
                f'style="left:{dx - 2}px"><span>the door</span></div>')
how_js.append(f'tl.fromTo("#door", {{opacity: 0}}, {{opacity: 1, duration: 0.2}}, {f(3 * BAR + 0.4)});\n')
# the 16th grid: one cell lit per 16th from the moment the engine node lands
for s in range(16 * 5):
    t = BAR + 0.4 + s * S16
    c, prev = s % 16, (s + 15) % 16
    how_js.append(f'tl.set("#g{c}", {{backgroundColor: "{LIME_INK}"}}, {f(t)});')
    how_js.append(f'tl.set("#g{prev}", {{backgroundColor: "#fff"}}, {f(t)});\n')
how_titles = [
    title("ho-t0", ["nine buttons. nine *wires.*"], 0, BAR, cls="paper", size=120),
    title("ho-t1", ["every press *snaps*", "to the 16th-note grid"], BAR, BAR, cls="paper", size=120),
    title("ho-t2", ["*30* *bytes* cross the door", "by radio. no router."], 2 * BAR, BAR, cls="paper", size=120),
    title("ho-t3", ["the inside box plays", "the *same* *notes*"], 3 * BAR, BAR, cls="paper", size=120),
    title("ho-t4", ["and the phone", "gets a *message*"], 4 * BAR, BAR, cls="paper", size=120),
    title("ho-t5", ["no *audio* crosses.", "same samples, both sides"], 5 * BAR, BAR, cls="paper", size=120),
]
how_css = f"""
    #root {{ background: {PAPER}; }}
    .node {{ position: absolute; top: {NY}px; width: {NW}px; height: {NH}px; border: 12px solid {LIME_INK}; border-radius: 18px;
             background: #fff; overflow: visible; transform-origin: 50% 50%; z-index: 20 !important; }}
    .node em {{ position: absolute; left: -12px; right: -12px; top: {NH - 12 + 18}px; text-align: center; font-style: normal;
             font-family: "Inter", sans-serif; font-weight: 700; font-size: 34px; color: {INK}; letter-spacing: 0.02em; }}
    .nodemedia {{ position: absolute; inset: 0; width: 100%; height: 100%; object-fit: cover; border-radius: 6px; }}
    .line {{ position: absolute; top: {NY + NH // 2 - 4}px; height: 8px; background: {LIME_INK}; transform-origin: 0 50%; z-index: 10 !important; }}
    .doormark {{ position: absolute; top: {NY - 40}px; width: 4px; height: {NH + 80}px; background: {INK}; z-index: 15 !important; }}
    .doormark span {{ position: absolute; left: -80px; width: 164px; top: -46px; text-align: center; font-family: "Inter", sans-serif;
             font-weight: 700; font-size: 28px; color: {INK}; }}
    .gridbox {{ position: absolute; inset: 0; padding: 18px; display: grid; grid-template-columns: repeat(4, 1fr); gap: 10px; align-content: start; }}
    .gridbox i {{ display: block; aspect-ratio: 1; background: #fff; border: 3px solid {INK}; border-radius: 6px; }}
    .gridbox b {{ grid-column: 1 / -1; margin-top: 14px; text-align: center; font-family: "League Gothic", sans-serif; font-weight: 400;
             font-size: 54px; text-transform: uppercase; color: {INK}; }}
    .radio {{ position: absolute; inset: 0; display: flex; flex-direction: column; align-items: center; justify-content: center; gap: 6px; }}
    .radio b {{ font-family: "League Gothic", sans-serif; font-weight: 400; font-size: 190px; line-height: 0.9; color: {INK}; }}
    .radio span {{ font-family: "Inter", sans-serif; font-weight: 700; font-size: 34px; color: {INK}; text-align: center; line-height: 1.15; }}
    .title.paper {{ left: 110px; width: 1700px; bottom: 70px; }}
"""
add("how", 8, 6, how_body + [("", "".join(how_js))] + how_titles, extra_css=how_css)

# ---------------------------------------------------------------- b14-21 BUILD
add("build", 14, 8, [
    photo("bu-p1", "p6770", 0, BAR, pos="50% 50%", push=1.10),
    video("bu-v2", "face_6898", BAR, BAR, media_start=1.0, pos="62% 40%"),
    video("bu-v3", "solder_6902", 2 * BAR, BAR, media_start=2.0, pos="50% 30%"),
    video("bu-v4", "tl_6911", 3 * BAR, BAR, media_start=3.0),
    video("bu-v5", "strip_6916", 4 * BAR, BAR, media_start=6.0),
    photo("bu-p6", "p7100", 5 * BAR, BAR, pos="50% 50%", push=1.10),
    video("bu-v7", "assy_6923a", 6 * BAR, BAR, media_start=10.0),
    video("bu-v8", "hero_6921", 7 * BAR, BAR, media_start=4.0),
    '<div class="shade"></div>',
    title("bu-t1", ["*24* mm hole.", "2.5 mm plate."], 0, BAR),
    title("bu-t2", ["*nine* arcade buttons"], BAR, BAR),
    title("bu-t3", ["nine wires.", "*one* ground."], 2 * BAR, BAR),
    title("bu-t4", ["*24* minutes", "of soldering"], 3 * BAR, BAR),
    title("bu-t5", ["three strips,", "lit from the *side*"], 4 * BAR, BAR),
    title("bu-t6", ["*first* light"], 5 * BAR, BAR),
    title("bu-t7", ["the *shell*"], 6 * BAR, BAR),
    title("bu-t8", ["first *fit*"], 7 * BAR, BAR),
])

# ---------------------------------------------------------------- b22-23 FIVE BOARDS
add("boards", 22, 2, [
    photo("bo-p1", "p7017", 0, BAR, pos="50% 45%", push=1.12),
    photo("bo-p2", "p7121", BAR, BAR, pos="50% 50%", push=1.08),
    '<div class="shade"></div>',
    title("bo-t1", ["*5* boards.", "1 survived."], 0, 2 * BAR, size=170),
], extra_css="\n    #bo-t1 .acc { color: #ff2bd6; }")

# ---------------------------------------------------------------- b24-27 INSTALL
add("install", 24, 4, [
    photo("in-p1", "p7107", 0, BAR, pos="50% 50%", push=1.08),
    photo("in-p2", "p7111", BAR, BAR, pos="50% 55%", push=1.08),
    video("in-v3", "shellwall_6927", 2 * BAR, BAR, media_start=1.0, pos="60% 50%"),
    video("in-v4", "finished_7157", 3 * BAR, BAR, media_start=0.3, pos="50% 40%"),
    '<div class="shade"></div>',
    title("in-t1", ["*breaker* off."], 0, BAR),
    title("in-t2", ["230 V in.", "*12* *V* out."], BAR, BAR),
    title("in-t3", ["12 V on the *pair*"], 2 * BAR, BAR),
    title("in-t4", ["on the *wall*"], 3 * BAR, BAR, size=170),
])

# ---------------------------------------------------------------- b28-33 PAYOFF
DOOR_T = 8.0
PL, PR = W // 2 - PW, W // 2          # two pillars touching at the centre
add("payoff", 28, 6, [
    pillar("pa", "door_7119", 0, 6 * BAR, media_start=DOOR_T, sound=2.0, fade_in=0.05, fade_out=0.15, left=1150),
    # bar 31: the inside box joins on the right; the door pillar is re-cut to the left half
    video("pa-in-bg", "finished_7157_bg", 3 * BAR, 3 * BAR, media_start=14.5, z=4),
    video("pa-door2", "door_7119", 3 * BAR, 3 * BAR, media_start=DOOR_T + 3 * BAR, z=5, cls="pillar")
        .replace('style="object-position:50% 50%;z-index:5"', f'style="left:{PL}px;z-index:5"'),
    video("pa-in", "finished_7157", 3 * BAR, 3 * BAR, media_start=14.5, z=5, cls="pillar")
        .replace('style="object-position:50% 50%;z-index:5"', f'style="left:{PR}px;z-index:5"'),
    '<div id="pa-shade" class="clip shade top" data-start="%s" data-duration="%s" data-track-index="4"></div>' % (f(2 * BAR), f(BAR)),
    title("pa-t1", ["every visitor", "is a *DJ*"], 2 * BAR, BAR, pos="top", size=190),
    title("pa-t2", ["the inside box", "answers, *30* *ms*", "behind"], 3 * BAR, 3 * BAR, pos="top", size=130, cls="narrow"),
], extra_css="\n    .title.narrow { width: 620px; left: 60px; top: 90px; }\n    #pa-shade { z-index: 40 !important; }")

# ---------------------------------------------------------------- b34-37 THE BOX AND THE PHONE
banner_html = """<div id="banner" class="clip" data-start="%s" data-duration="%s" data-track-index="6" data-layout-allow-overflow>
      <div class="ic">DJ</div><div class="tx"><b>DJ Doorbell</b><span>Someone is playing the doorbell</span></div><div class="tm">now</div>
    </div>""" % (f(3 * BAR), f(BAR))
banner_js = (f'tl.fromTo("#banner", {{y: -160}}, {{y: 0, duration: 0.27, ease: "power3.out"}}, {f(3 * BAR)});\n')
add("box", 34, 4, [
    photo("bx-p1", "p7155", 0, BAR, pos="50% 50%", push=1.08),
    photo("bx-p2", "p7156", BAR, BAR, pos="50% 45%", push=1.08),
    pillar("bx", "phone_7123", 2 * BAR, 2 * BAR, media_start=3.0, left=1150),
    '<div class="shade"></div>',
    title("bx-t1", ["the chime got", "its own *box*"], 0, BAR),
    title("bx-t2", ["and a *knob*"], BAR, BAR, size=170),
    title("bx-t3", ["quiet hours", "after *22:00*"], 2 * BAR, BAR, cls="narrow"),
    title("bx-t4", ["and a *message*", "to my phone"], 3 * BAR, BAR, cls="narrow"),
    (banner_html, banner_js),
], extra_css=f"""
    .title.narrow {{ width: 1000px; }}
    #banner {{ position: absolute; left: 1182px; top: 36px; width: 544px; height: 128px; z-index: 70 !important; background: rgba(250,250,250,0.96);
               border-radius: 28px; display: flex; align-items: center; gap: 18px; padding: 0 22px; box-shadow: 0 8px 30px rgba(0,0,0,0.35); }}
    #banner .ic {{ width: 76px; height: 76px; border-radius: 18px; background: {LIME}; color: {INK}; font-family: "League Gothic", sans-serif;
               font-size: 44px; display: grid; place-items: center; flex: none; }}
    #banner .tx {{ flex: 1; min-width: 0; font-family: "Inter", sans-serif; color: #111; line-height: 1.2; }}
    #banner .tx b {{ display: block; font-size: 30px; font-weight: 700; }}
    #banner .tx span {{ display: block; font-size: 28px; font-weight: 400; white-space: nowrap; }}
    #banner .tm {{ font-family: "Inter", sans-serif; font-size: 28px; color: #666; align-self: flex-start; margin-top: 18px; }}
""")

# ---------------------------------------------------------------- b38-39 CLAIM
add("claim", 38, 2, [
    pillar("cl", "finished_7157", 0, 2 * BAR, media_start=9.5, left=1150),
    '<div class="shade top"></div>',
    title("cl-t", ["a doorbell", "you *play,*", "not press"], 0, 2 * BAR, pos="top", size=190),
])

# ---------------------------------------------------------------- b40-43 END CARD
end_css = f"""
    #end-field {{ position: absolute; inset: 0; background: {LIME}; z-index: 20 !important; }}
    #end-name {{ position: absolute; left: 110px; top: 250px; z-index: 30 !important; font-family: "League Gothic", sans-serif;
                 font-size: 360px; line-height: 0.86; text-transform: uppercase; color: {INK}; }}
    #end-link {{ position: absolute; left: 110px; top: 900px; z-index: 30 !important; font-family: "Inter", sans-serif;
                 font-weight: 700; font-size: 52px; color: {INK}; letter-spacing: 0.01em; }}
    #end-sub {{ position: absolute; left: 110px; top: 150px; z-index: 30 !important; font-family: "Inter", sans-serif;
                 font-weight: 700; font-size: 40px; color: {INK}; letter-spacing: 0.12em; text-transform: uppercase; }}
"""
end_js = (
    'tl.fromTo("#end-field", {y: 1080}, {y: 0, duration: 0.4, ease: "power3.inOut"}, 0);\n'
    'tl.fromTo("#end-name .w", {opacity: 0, y: 40}, {opacity: 1, y: 0, duration: 0.3, stagger: 0.08, ease: "power4.out"}, 0.3);\n'
    'tl.fromTo("#end-sub", {opacity: 0}, {opacity: 1, duration: 0.2, ease: "power2.out"}, 0.55);\n'
    'tl.fromTo("#end-link", {opacity: 0}, {opacity: 1, duration: 0.2, ease: "power2.out"}, 0.7);\n'
)
add("end", 40, 4, [
    pillar("en", "finished_7157", 0, 0.5 * BAR, media_start=9.5 + 2 * BAR, left=1150),
    f'<div id="end-field" class="clip" data-start="0" data-duration="{f(4 * BAR)}" data-track-index="3" data-layout-allow-overflow></div>',
    f'<div id="end-sub" class="clip" data-start="0" data-duration="{f(4 * BAR)}" data-track-index="5">open source · esp32 · 3d printed</div>',
    f'<div id="end-name" class="clip" data-start="0" data-duration="{f(4 * BAR)}" data-track-index="5"><div class="ln"><span class="w">DJ</span> <span class="w">Doorbell</span></div></div>',
    f'<div id="end-link" class="clip" data-start="0" data-duration="{f(4 * BAR)}" data-track-index="5">github.com/andzhelo/DjDoorbell</div>',
    (" ", end_js),
], extra_css=end_css)

# ---------------------------------------------------------------- write
comp = HERE / "compositions"
comp.mkdir(exist_ok=True)
hosts = []
for id_, sb, bars, html in scenes:
    (comp / f"{id_}.html").write_text(html)
    hosts.append(f'    <div id="{id_}" class="clip" data-composition-src="compositions/{id_}.html" data-composition-id="{id_}" '
                 f'data-start="{f(sb * BAR)}" data-duration="{f(bars * BAR)}" data-track-kind="graphics" data-track-index="0"></div>')

index = f"""<!doctype html>
<html lang="en" data-composition-variables='[
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
    <audio id="music1" src="assets/audio/track.wav" data-start="{f(4 * BAR)}" data-media-start="{f(4 * BAR)}" data-duration="{f(24 * BAR)}" data-volume="1" data-fade-in="0.2" data-fade-out="0.3" data-track-index="8"></audio>
    <audio id="music2" src="assets/audio/track.wav" data-start="{f(34 * BAR)}" data-media-start="{f(34 * BAR)}" data-duration="{f(10 * BAR)}" data-volume="1" data-fade-in="0.2" data-fade-out="2.0" data-track-index="8"></audio>
    <audio id="chime" src="assets/audio/chime.wav" data-start="{f(40 * BAR + BEAT)}" data-volume="0.55" data-track-index="9"></audio>
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
    print(f"  {id_:8s} b{sb:<2d} {bars} bar(s)  {sb*BAR:6.2f}-{(sb+bars)*BAR:6.2f} s")
