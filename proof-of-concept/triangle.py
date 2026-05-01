#!/usr/bin/python
import matplotlib
from matplotlib import pyplot as plt
matplotlib.use('qtagg')

N = 4096
points = []

for i in range(N):
    value = 1.0 - 4.0 * abs((i / N) - 0.5)
    asInt = int(value * 32767)

    points.append((i, asInt))

x, y = zip(*points)

plt.plot(x, y)
plt.xlabel("Sample")
plt.ylabel("Amplitude")
plt.title("Triangle")
plt.show()