"""Generate short MIT-licensed cues, scheduled by the session clock."""
import math
import pathlib
import struct
import wave

root = pathlib.Path(__file__).resolve().parents[1] / "assets"
rate = 48000
for name, frequency in (("countdown-beat.wav", 660), ("countdown-end.wav", 880)):
    samples = []
    for frame in range(int(rate * 0.24)):
        t = frame / rate
        envelope = min(1, t / 0.002) * math.exp(-t * 24)
        tone = math.sin(2 * math.pi * frequency * t) + 0.2 * math.sin(2 * math.pi * frequency * 2 * t)
        samples.append(struct.pack("<h", int(8500 * envelope * tone)))
    with wave.open(str(root / name), "wb") as output:
        output.setnchannels(1)
        output.setsampwidth(2)
        output.setframerate(rate)
        output.writeframes(b"".join(samples))
