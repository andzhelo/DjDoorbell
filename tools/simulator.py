"""
DJ Doorbell — pad simulator.

Mirrors the firmware exactly: one looping bed voice, nine retriggerable
one-shot voices, a sample-accurate clock taken from the audio callback (never
wall-clock time), and presses quantised to the 16th grid.

    python3 simulator.py            live, clickable + keyboard
    python3 simulator.py --render   offline, writes simulation.wav (no audio device needed)

Live mode needs sounddevice:  pip3 install sounddevice numpy
Render mode needs only numpy.
"""
import sys, os, wave
import numpy as np

SR   = 22050
BPM  = 124.0
BEAT = 60.0 / BPM
S16  = BEAT / 4                     # 16th note = the quantise grid
BLOCK = 256                         # callback size; ~11.6 ms

HERE = os.path.dirname(os.path.abspath(__file__))
RAW  = os.path.join(os.path.dirname(HERE), "data", "raw")

PADS = [
    ("kick",     "01_kick",     "#E09030"),
    ("clap",     "02_clap",     "#E09030"),
    ("hat",      "03_hat",      "#E09030"),
    ("Am",       "04_stab_am",  "#37AFC4"),
    ("C",        "05_stab_c",   "#37AFC4"),
    ("F",        "06_stab_f",   "#37AFC4"),
    ("sub",      "07_sub",      "#C43F81"),
    ("pluck",    "08_pluck",    "#C43F81"),
    ("riser",    "09_riser",    "#C43F81"),
]
KEYS = "qweasdzxc"
VARIANTS = {2: "10_hat_open"}       # pad index -> sample on a rapid repeat
REPEAT_WINDOW = 2                   # in 16ths


def load(name):
    path = os.path.join(RAW, name + ".raw")
    if not os.path.exists(path):
        sys.exit("missing %s — run make_samples.py first" % path)
    return np.frombuffer(open(path, "rb").read(), dtype="<i2").astype(np.float32) / 32768.0


# --------------------------------------------------------------------------
class Engine:
    """Backend-agnostic mixer. process(frames) returns one block of float32."""

    def __init__(self):
        self.samples = [load(f) for _, f, _ in PADS]
        self.variants = {i: load(f) for i, f in VARIANTS.items()}
        self.last_pad = {}          # pad -> sample time of its previous press
        self.loop = load("00_loop")
        self.loop_len = len(self.loop)

        self.now = 0                # master sample counter — the only clock
        self.voices = []            # active one-shots
        self.pending = []           # (target_sample, pad_index)
        self.bed_on = True
        self.quantise = True
        self.bed_pos = None         # None = bed not running
        self.last_press = -10 ** 9
        self.fired = []             # (pad, sample) — for the UI to flash on

    # ---- control -------------------------------------------------------
    def next_grid(self):
        """Sample index of the next unplayed 16th boundary."""
        if not self.quantise or self.bed_pos is None:
            return self.now
        g = int(S16 * SR)
        # grid is anchored to where the bed started
        phase = (self.now - self.bed_start) % g
        return self.now + (g - phase)

    def press(self, idx):
        if self.bed_pos is None and self.bed_on:
            self.bed_pos = 0
            self.bed_start = self.now
        self.last_press = self.now
        g = int(S16 * SR)
        repeat = idx in self.last_pad and self.now - self.last_pad[idx] < REPEAT_WINDOW * g
        self.last_pad[idx] = self.now
        data = self.variants[idx] if repeat and idx in self.variants else self.samples[idx]
        self.pending.append((self.next_grid(), idx, data))
        self.pending.sort(key=lambda p: p[0])

    def toggle_bed(self):
        self.bed_on = not self.bed_on
        if not self.bed_on:
            self.bed_pos = None

    # ---- audio ---------------------------------------------------------
    def process(self, frames):
        buf = np.zeros(frames, dtype=np.float32)

        # bed voice, wrapping
        if self.bed_pos is not None:
            written = 0
            while written < frames:
                n = min(frames - written, self.loop_len - self.bed_pos)
                buf[written:written + n] += self.loop[self.bed_pos:self.bed_pos + n] * 0.80
                self.bed_pos = (self.bed_pos + n) % self.loop_len
                written += n

        # start any one-shots that land inside this block
        while self.pending and self.pending[0][0] < self.now + frames:
            target, idx, data = self.pending.pop(0)
            delay = max(0, target - self.now)
            self.voices.append([data, 0, delay])
            self.fired.append((idx, target))

        # mix active one-shots
        alive = []
        for v in self.voices:
            data, pos, delay = v
            if delay >= frames:
                v[2] -= frames
                alive.append(v)
                continue
            start = delay
            n = min(frames - start, len(data) - pos)
            if n > 0:
                buf[start:start + n] += data[pos:pos + n] * 0.85
                v[1] = pos + n
                v[2] = 0
            if v[1] < len(data):
                alive.append(v)
        self.voices = alive

        # bed stops after 8 s of no presses
        if self.bed_pos is not None and self.now - self.last_press > 8 * SR:
            self.bed_pos = None

        self.now += frames
        return np.tanh(buf * 0.62)             # headroom + soft limit, never clips


# --------------------------------------------------------------------------
def render(seconds=16.0, out="simulation.wav"):
    """Offline: scripted presses through the same engine. No audio device."""
    e = Engine()
    script = [(step, idx) for step, idx in [
        (0, 3), (6, 7), (10, 3), (16, 4), (20, 7), (22, 7), (26, 1),
        (32, 5), (34, 7), (38, 4), (44, 8), (48, 3), (52, 7), (54, 7),
        (56, 5), (60, 4), (62, 7), (64, 3), (68, 7), (72, 5), (76, 4),
        (80, 3), (84, 7), (88, 7), (92, 8), (96, 3),
    ]]
    # deliberately sloppy timing — 18 ms early or late, like a real finger
    jitter = (np.random.default_rng(3).random(len(script)) - 0.5) * 0.036
    events = sorted((step * S16 + j, idx) for (step, idx), j in zip(script, jitter))

    total = int(seconds * SR)
    blocks = []
    ei = 0
    while e.now < total:
        while ei < len(events) and events[ei][0] * SR <= e.now:
            e.press(events[ei][1])
            ei += 1
        blocks.append(e.process(BLOCK).copy())

    sig = np.concatenate(blocks)[:total]
    sig = sig / max(1e-9, np.abs(sig).max()) * 0.89
    pcm = (sig * 32767).astype("<i2")
    with wave.open(os.path.join(HERE, out), "wb") as w:
        w.setnchannels(1); w.setsampwidth(2); w.setframerate(SR)
        w.writeframes(pcm.tobytes())
    print("wrote %s — %.1f s" % (out, len(pcm) / SR))


# --------------------------------------------------------------------------
def live():
    try:
        import sounddevice as sd
    except ImportError:
        sys.exit("pip3 install sounddevice   (or use --render)")
    import tkinter as tk

    e = Engine()

    def callback(outdata, frames, t, status):
        outdata[:, 0] = e.process(frames)

    stream = sd.OutputStream(samplerate=SR, channels=1, dtype="float32",
                             blocksize=BLOCK, callback=callback)
    stream.start()

    root = tk.Tk()
    root.title("DJ doorbell — pad simulator")
    root.configure(bg="#15171B")

    cv = tk.Canvas(root, width=330, height=330, bg="#23272E",
                   highlightthickness=1, highlightbackground="#454A54")
    cv.pack(padx=18, pady=(18, 8))

    caps, labels = [], []
    for i, (name, _, colour) in enumerate(PADS):
        r, c = divmod(i, 3)
        cx, cy = 55 + c * 110, 55 + r * 110
        cv.create_oval(cx - 42, cy - 42, cx + 42, cy + 42, fill="#1A1D22", outline="")
        cap = cv.create_oval(cx - 38, cy - 38, cx + 38, cy + 38,
                             fill="#C9C6BE", outline="#8F8C85")
        cv.create_text(cx, cy + 4, text=name, fill="#5A5952",
                       font=("Helvetica", 12, "bold"))
        cv.create_text(cx, cy + 24, text=KEYS[i].upper(), fill="#8A8781",
                       font=("Helvetica", 9))
        caps.append(cap); labels.append(colour)
        cv.tag_bind(cap, "<Button-1>", lambda ev, k=i: hit(k))

    def flash(i):
        cv.itemconfig(caps[i], fill=labels[i])
        root.after(110, lambda: cv.itemconfig(caps[i], fill="#C9C6BE"))

    def hit(i):
        e.press(i)
        delay = max(0, (e.pending[-1][0] - e.now)) / SR if e.pending else 0
        root.after(int(delay * 1000), lambda: flash(i))

    bar = tk.Frame(root, bg="#15171B")
    bar.pack(pady=(0, 16))

    qv = tk.BooleanVar(value=True)
    bv = tk.BooleanVar(value=True)

    def set_q():
        e.quantise = qv.get()

    def set_b():
        e.toggle_bed()

    for text, var, cmd in (("quantise", qv, set_q), ("beat bed", bv, set_b)):
        tk.Checkbutton(bar, text=text, variable=var, command=cmd,
                       bg="#15171B", fg="#C9C6BE", selectcolor="#23272E",
                       activebackground="#15171B", activeforeground="#E6E2D8",
                       font=("Helvetica", 11)).pack(side="left", padx=10)

    tk.Label(bar, text="%.0f BPM" % BPM, bg="#15171B", fg="#6E737B",
             font=("Helvetica", 11)).pack(side="left", padx=10)

    for i, k in enumerate(KEYS):
        root.bind(k, lambda ev, x=i: hit(x))
        root.bind(k.upper(), lambda ev, x=i: hit(x))

    root.protocol("WM_DELETE_WINDOW", lambda: (stream.stop(), root.destroy()))
    root.mainloop()


if __name__ == "__main__":
    if "--render" in sys.argv:
        render()
    else:
        live()

