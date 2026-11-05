#!/usr/bin/env python3
"""The film's music: a scripted 44-bar performance through the doorbell's own engine
(tools/simulator.py), so every cut sits on the engine's grid. Writes assets/audio/track.wav.
Run from the repo root: /usr/bin/python3 videos/djdoorbell-yt/make_track.py"""
import os, sys, wave
import numpy as np

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(ROOT, "tools"))
import simulator as sim

G = int(sim.S16 * sim.SR)          # 2667 samples per 16th
BARS = 44
KICK, CLAP, HAT, AM, C, F, SUB, PLUCK, RISER = range(9)

events = {}   # step -> [pads]
def p(bar, step, pad):
    events.setdefault(bar * 16 + step, []).append(pad)

def four_floor(b):
    for s in (0, 4, 8, 12): p(b, s, KICK)
def claps(b):
    for s in (4, 12): p(b, s, CLAP)
def hats8(b):
    for s in range(2, 16, 4): p(b, s, HAT)
def hats16(b):
    for s in range(0, 16, 2): p(b, s, HAT)

# bars 0-3: under the hook (not played in the film, keeps the bed alive and phase-locked)
for b in range(0, 4): four_floor(b); hats8(b)

# bars 4-7: the wall. Sparse: kick, offbeat hat, one sub.
for b in range(4, 8):
    four_floor(b); hats8(b)
p(5, 0, SUB); p(7, 0, SUB); p(7, 8, AM)

# bars 8-13: the diagram. Steady, one stab per bar as each node lands.
for b in range(8, 14):
    four_floor(b); hats8(b); claps(b)
p(8, 0, AM); p(9, 0, C); p(10, 0, F); p(11, 0, PLUCK); p(11, 4, PLUCK); p(12, 0, AM); p(12, 8, PLUCK)
p(13, 0, SUB); p(13, 12, RISER)

# bars 14-21: the build. Dense.
for b in range(14, 22):
    four_floor(b); claps(b); hats16(b)
    p(b, 0, [AM, C, F, AM][(b - 14) % 4])
    p(b, 8, [PLUCK, SUB][(b - 14) % 2])
    if b % 2: p(b, 14, HAT); p(b, 15, HAT)   # open hat flourish
p(17, 6, PLUCK); p(17, 10, PLUCK); p(19, 6, C); p(19, 12, F); p(21, 8, RISER)

# bars 22-23: five boards. Thin: sub and hat only, riser into install.
p(22, 0, SUB); p(22, 8, HAT); p(22, 12, HAT); p(23, 0, SUB); p(23, 4, RISER)

# bars 24-27: install. Builds back up, riser into the payoff.
for b in range(24, 28):
    four_floor(b); hats8(b)
    if b >= 25: claps(b)
p(24, 0, AM); p(25, 0, C); p(26, 0, F); p(26, 8, PLUCK); p(27, 0, AM); p(27, 4, RISER)

# bars 28-33: under the payoff (real door audio plays instead; keep the bed running)
for b in range(28, 34): four_floor(b); hats8(b)

# bars 34-37: the box and the phone. Mid.
for b in range(34, 38):
    four_floor(b); claps(b); hats8(b)
    p(b, 0, [AM, PLUCK, C, PLUCK][b - 34])
p(37, 12, RISER)

# bars 38-39: the claim. Full.
for b in (38, 39):
    four_floor(b); claps(b); hats16(b); p(b, 0, AM); p(b, 8, SUB)
p(39, 12, F)

# bars 40-43: end card. Bed and plucks, thinning.
for b in range(40, 44):
    four_floor(b); hats8(b)
p(40, 0, PLUCK); p(41, 0, PLUCK); p(42, 0, AM)

e = sim.Engine()
e.quantise = False
e.press(KICK)            # step 0 lands at sample 0
e.quantise = True
out = []
total_steps = BARS * 16
for k in range(total_steps):
    # press now for everything due on step k+1: the engine snaps it to the next grid line
    for pad in events.get(k + 1, []):
        e.press(pad)
    if k == 0:
        for pad in events.get(0, []):
            if pad != KICK: e.quantise = False; e.press(pad); e.quantise = True
    out.append(e.process(G))
    e.last_press = e.now    # never let the bed time out
y = np.concatenate(out)
y = y / max(1e-6, np.abs(y).max()) * 0.89
path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "assets", "audio", "track.wav")
with wave.open(path, "wb") as w:
    w.setnchannels(1); w.setsampwidth(2); w.setframerate(sim.SR)
    w.writeframes((y * 32767).astype("<i2").tobytes())
print("wrote %s: %.2f s, %d bars, bar %.5f s" % (path, len(y) / sim.SR, BARS, 16 * G / sim.SR))
