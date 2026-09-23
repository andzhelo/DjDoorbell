"""
DJ Doorbell — sample set generator.
House/techno, ~124 BPM, A minor.

Outputs raw 16-bit PCM mono @ 22050 Hz (the format the firmware reads from
flash) to data/raw, plus .wav previews of the same audio in data/preview so
you can audition on a laptop.
"""
import numpy as np, wave, os

SR   = 22050
# The firmware quantises to an integer grid of 2667 samples (the 16th at
# 124 BPM, truncated). Build everything on that exact grid so the 4-bar loop
# is precisely 64 grid steps and never drifts against the quantiser.
# Nominal tempo comes out at 124.015 BPM.
GRID = 2667
S16  = GRID / SR
BEAT = 4 * S16
BAR  = 4 * BEAT
BPM  = 60.0 / BEAT

HERE = os.path.dirname(os.path.abspath(__file__))
OUT  = os.path.join(os.path.dirname(HERE), "data")
RAW  = os.path.join(OUT, "raw")
PRE  = os.path.join(OUT, "preview")
for d in (RAW, PRE):
    os.makedirs(d, exist_ok=True)

rng = np.random.default_rng(11)
hz = lambda n: 440.0 * 2 ** ((n - 69) / 12.0)


# ---------------------------------------------------------------- helpers
def lp(sig, cutoff):
    """one-pole lowpass; cutoff is an array of coefficients 0..1"""
    y = np.zeros(len(sig)); prev = 0.0
    for i in range(len(sig)):
        prev += cutoff[i] * (sig[i] - prev)
        y[i] = prev
    return y


def hp(sig, times=3, a=0.92):
    for _ in range(times):
        sig = np.convolve(sig, [1, -a], mode="same")
    return sig


def saw(t, f):
    return 2 * ((t * f) % 1.0) - 1.0


def env(n, atk, dec, curve=5.0):
    a = max(1, int(atk * SR))
    e = np.exp(-np.linspace(0, curve, max(1, n - a)))
    return np.concatenate([np.linspace(0, 1, a), e])[:n]


# ---------------------------------------------------------------- voices
def kick(dur=0.52):
    n = int(dur * SR); t = np.arange(n) / SR
    # two-stage pitch envelope: snap, then settle to the fundamental
    f = 47 + 30 * np.exp(-t * 12) + 170 * np.exp(-t * 55)
    body = np.sin(2 * np.pi * np.cumsum(f) / SR) * np.exp(-t * 6.5)
    click = hp(rng.normal(0, 1, n), 2) * np.exp(-t * 500) * 0.30
    return np.tanh((body + click) * 1.9) * 0.95


def clap(dur=0.42):
    n = int(dur * SR); out = np.zeros(n)
    for k, off in enumerate((0.0, 0.010, 0.021, 0.033)):
        i = int(off * SR); m = n - i
        t = np.arange(m) / SR
        nz = hp(rng.normal(0, 1, m), 2, 0.86)
        decay = 55.0 if k < 3 else 11.0          # last burst is the tail
        out[i:] += nz * np.exp(-t * decay) * (0.55 if k < 3 else 1.0)
    band = lp(out, np.full(n, 0.55))             # tame the very top
    return band * 0.52


def hat(dur=0.055, open_=False):
    d = 0.30 if open_ else dur
    n = int(d * SR); t = np.arange(n) / SR
    nz = hp(rng.normal(0, 1, n), 4, 0.95)
    a = np.exp(-t * (11 if open_ else 105))
    if open_:                                     # slight shimmer on the tail
        a *= 1 + 0.18 * np.sin(2 * np.pi * 47 * t)
    return nz * a * 0.34


def stab(notes, dur=0.62):
    """House organ stab — additive, hard attack, two detuned layers."""
    n = int(dur * SR); t = np.arange(n) / SR
    out = np.zeros(n)
    for nt in notes:
        f = hz(nt)
        for h, amp in ((1, 1.0), (2, 0.55), (3, 0.28), (4, 0.16), (6, 0.09)):
            out += np.sin(2 * np.pi * f * h * t) * amp
            out += np.sin(2 * np.pi * f * h * 1.004 * t) * amp * 0.6
    out /= len(notes) * 3.2
    e = np.exp(-t * 5.2) * (1 - np.exp(-t * 900))
    cut = 0.55 * np.exp(-t * 7) + 0.10
    return lp(out * e, cut) * 0.60


def sub(note=45, dur=0.60):
    """Bass hit. A2 (110 Hz) with heavy drive: neither speaker reproduces
    55 Hz, so the 2nd-4th harmonics have to carry the note."""
    n = int(dur * SR); t = np.arange(n) / SR
    f = hz(note) * (1 + 0.35 * np.exp(-t * 40))
    s = np.sin(2 * np.pi * np.cumsum(f) / SR)
    return np.tanh(s * 2.6) * env(n, 0.004, 0, 4.5) * 0.72


def pluck(note=69, dur=0.34):
    n = int(dur * SR); t = np.arange(n) / SR
    f = hz(note)
    sig = saw(t, f) * 0.7 + np.sign(np.sin(2 * np.pi * f * t)) * 0.3
    cut = 0.72 * np.exp(-t * 16) + 0.05
    return lp(sig, cut) * env(n, 0.002, 0, 7) * 0.52


def riser(dur=1.10):
    n = int(dur * SR); t = np.arange(n) / SR
    nz = rng.normal(0, 1, n)
    cut = np.linspace(0.015, 0.62, n)
    tone = np.sin(2 * np.pi * np.cumsum(np.linspace(180, 1400, n)) / SR) * 0.25
    return (lp(nz, cut) + tone) * (t / dur) ** 2.2 * 0.42


# ---------------------------------------------------------------- pad map
# Row colours from the panel design: amber drums, cyan tone, magenta fx.
PADS = [
    ("01_kick",     kick()),
    ("02_clap",     clap()),
    ("03_hat",      hat()),
    ("04_stab_am",  stab([57, 60, 64])),        # A  C  E
    ("05_stab_c",   stab([60, 64, 67])),        # C  E  G
    ("06_stab_f",   stab([65, 69, 72])),        # F  A  C
    ("07_sub",      sub(45)),                   # A2
    ("08_pluck",    pluck(76)),                 # E4
    ("09_riser",    riser()),
]

# Variants: played instead of the pad's sample on a rapid repeat.
VARIANTS = [
    ("10_hat_open", hat(open_=True)),           # pad 3 repeat
]


# ---------------------------------------------------------------- the loop
def build_loop(bars=4):
    n = int(bars * BAR * SR) + SR
    buf = np.zeros(n)

    def put(sig, t, g=1.0):
        i = int(t * SR); e = min(n, i + len(sig))
        buf[i:e] += sig[:e - i] * g

    k, c, oh, ch = kick(), clap(), hat(open_=True), hat()
    bassline = [(0, 45), (6, 45), (10, 48), (14, 52)]   # A A C E

    for b in range(bars):
        base = b * BAR
        last = b == bars - 1
        for beat in range(4):                            # four on the floor
            if not (last and beat == 3):                 # bar 4: drop the last kick
                put(k, base + beat * BEAT, 1.0)
            put(oh, base + beat * BEAT + BEAT / 2, 0.42) # offbeat open hat
        for beat in (1, 3):
            put(c, base + beat * BEAT, 0.62)
        if last:                                         # bar 4: clap flam into the turnaround
            put(c, base + 14 * S16, 0.45)
            put(c, base + 15 * S16, 0.55)
        for s in range(0, 16, 2):
            if s % 4 != 2:
                put(ch, base + s * S16, 0.20)
        for s, nt in bassline:
            put(sub(nt, 0.42), base + s * S16, 0.55)

    loop = buf[:int(bars * BAR * SR)]
    # crossfade the overhang back over the head so it loops seamlessly
    tail = buf[int(bars * BAR * SR):]
    loop[:len(tail)] += tail
    return loop


# ---------------------------------------------------------------- writing
def norm(x, peak=0.89):
    m = np.abs(x).max()
    return x / m * peak if m > 0 else x


def write_pair(name, sig):
    sig = norm(sig)
    pcm = (sig * 32767).astype("<i2")
    with open(os.path.join(RAW, name + ".raw"), "wb") as f:
        f.write(pcm.tobytes())
    with wave.open(os.path.join(PRE, name + ".wav"), "wb") as w:
        w.setnchannels(1); w.setsampwidth(2); w.setframerate(SR)
        w.writeframes(pcm.tobytes())
    return len(pcm)


total = 0
manifest = []
for name, sig in PADS + VARIANTS:
    ln = write_pair(name, sig)
    total += ln * 2
    manifest.append((name, ln / SR, ln * 2))

loop = build_loop(4)
assert len(loop) == 64 * GRID, len(loop)
ln = write_pair("00_loop", loop)
total += ln * 2
manifest.insert(0, ("00_loop", ln / SR, ln * 2))


# ---------------------------------------------------------------- demo mix
def demo():
    """What a visitor actually hears: bed running, presses quantised on top."""
    dur = 14.0
    n = int(dur * SR)
    buf = np.zeros(n + SR)
    lp_len = len(loop)

    def put(sig, t, g=1.0):
        i = int(t * SR)
        if i >= len(buf):
            return
        e = min(len(buf), i + len(sig))
        buf[i:e] += sig[:e - i] * g

    # bed enters on the first press and repeats
    for r in range(3):
        put(loop, r * lp_len / SR, 0.80)

    S = S16
    presses = [
        (0,  "04_stab_am"), (6,  "08_pluck"), (10, "04_stab_am"),
        (16, "05_stab_c"),  (20, "08_pluck"), (22, "08_pluck"), (26, "02_clap"),
        (32, "06_stab_f"), (34, "08_pluck"), (38, "05_stab_c"), (44, "09_riser"),
        (48, "04_stab_am"), (52, "08_pluck"), (54, "08_pluck"), (56, "06_stab_f"),
        (60, "05_stab_c"),  (62, "08_pluck"),
        (64, "04_stab_am"), (68, "08_pluck"), (72, "06_stab_f"), (76, "05_stab_c"),
        (80, "04_stab_am"), (84, "08_pluck"), (88, "08_pluck"), (92, "09_riser"),
        (96, "04_stab_am"),
    ]
    table = dict(PADS)
    for step, name in presses:
        put(norm(table[name]), step * S, 0.80)

    out = np.tanh(buf[:n] * 0.62)                    # same soft limit as the firmware
    return norm(out)


d = demo()
pcm = (d * 32767).astype("<i2")
with wave.open(os.path.join(PRE, "doorbell-house-demo.wav"), "wb") as w:
    w.setnchannels(1); w.setsampwidth(2); w.setframerate(SR)
    w.writeframes(pcm.tobytes())

with open(os.path.join(RAW, "manifest.txt"), "w") as f:
    f.write("DJ doorbell sample set — generated by tools/make_samples.py\n")
    f.write("raw 16-bit PCM mono, little-endian, %d Hz, %.3f BPM, A minor, grid %d samples\n\n" % (SR, BPM, GRID))
    for name, secs, byts in manifest:
        f.write("%-14s %5.2f s  %7d bytes\n" % (name, secs, byts))
    f.write("\ntotal %d bytes (%.0f KB)\n" % (total, total / 1024))

print("total flash needed: %.0f KB" % (total / 1024))
for name, secs, byts in manifest:
    print("  %-14s %5.2fs  %6d B" % (name, secs, byts))
