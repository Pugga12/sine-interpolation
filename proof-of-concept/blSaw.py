#!/usr/bin/python
import matplotlib
from matplotlib import pyplot as plt
import math

freq = 440
sample_rate = 44100
duration = 0.1
matplotlib.use('qtagg')

numSamples = int(sample_rate * duration)
t = [i / sample_rate for i in range(numSamples)]

def blSawtooth(tA, frequency, sampleRate):
    max_harmonic = int(sample_rate / (2 * frequency));
    signal = [0.0] * len(tA)

    for k in range(1, max_harmonic):
        for i in range(len(tA)):
            harmonic = math.sin(2 * math.pi * freq * tA[i] * k) / k
            signal[i] += harmonic

    for i in range(len(signal)):
        signal[i] *= 2 / math.pi

    return signal

y = blSawtooth(t, freq, sample_rate)

plt.plot(t, y)
plt.xlabel("Time (s)")
plt.ylabel("Amplitude")
plt.show()