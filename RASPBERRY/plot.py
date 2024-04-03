import matplotlib.pyplot as plt
import numpy as np
import math, time

def draw_text(fig, Y):
    arr = list()
    arr.append(fig.text(0.005, 0.15, f"MEAN: {beautify(np.mean(Y))}"))
    arr.append(fig.text(0.005, 0.12, f"MEDIAN: {beautify(np.median(Y))}"))
    arr.append(fig.text(0.005, 0.09, f"MAX: {beautify(np.max(Y))}"))
    arr.append(fig.text(0.005, 0.06, f"MIN: {beautify(np.min(Y))}"))
    arr.append(fig.text(0.005, 0.03, f"STD: {beautify(np.std(Y))}"))
    return arr

def remove_text(arr):
    for t in arr:
        t.set_visible(False)

def beautify(num):
    return f'{round(num, 3):.3f}'

def dbfft(rfft, ref):
    s_mag = np.abs(rfft)
    s_dbfs = 20 * np.log10(s_mag/ref)
    return s_dbfs

# def dbfft(X, ref=32768):
#     return 20 * np.log10(X/ref)

Fs = 1000
T = 1 / Fs
L = 1000
t = np.arange(0, L - 1) * T
pi = math.pi

X = 256 + 128 * np.sin(2 * pi * 50 * t) + 64 * np.sin(2 * pi * 120 * t) + 32 * np.sin(2 * pi * 350 * t)

print('MEAN:', beautify(np.mean(X)))
print('MEDIAN:', beautify(np.median(X)))
print('MAX:', beautify(np.max(X)))
print('MIN:', beautify(np.min(X)))

# Y = np.fft.fft(X)
# t = np.arange(0, L - 1)
# plt.plot(Fs/L*t, abs(Y))
# plt.title("Abs Magnitude of fft Spectrum")
# plt.xlabel("f (Hz)")
# plt.ylabel("|fft(X)|")

fig, (ax1, ax2, ax3) = plt.subplots(3, 1)
for a in [ax1, ax2, ax3]:
    a.grid()
    a.locator_params(axis='x', nbins=30)
    a.locator_params(axis='y', nbins=10)

ax1.plot(Fs*t, X)
ax1.set_title("Signal")
ax1.set_xlabel("Time [ms]")
ax1.set_ylabel("Voltage")

Y = 2 * np.fft.rfft(X)
t = np.arange(1, L/2)
ax2.plot(Fs/L*t, abs(Y)[1:] / 1000)
ax2.set_title("Amplitude Spectrum")
ax2.set_xlabel("Frequency [Hz]")
ax2.set_ylabel("Amplitude, 10^3")

Y = dbfft(Y, 20)
t = np.arange(1, L // 2)
ax3.plot(t, Y[1:])
ax3.set_title("Decibels")
ax3.set_xlabel("Frequency [Hz]")
ax3.set_ylabel("Intensity [dB]")
fig.set_size_inches(14, 8)
fig.savefig('test.jpg', dpi=150)
# set_box_aspect
arr = list()
while True:
    t = np.arange(0, L - 1) * T
    X = 256 + 128 * np.sin(2 * pi * 50 * t) + 64 * np.sin(2 * pi * 120 * t)
    remove_text(arr)
    ax1.clear()
    arr = draw_text(fig, X)
    ax1.plot(Fs*t, X)
    plt.pause(0.0001)
    # plt.figtext(0.02, 0.05, "typeeeee")
    # plt.clf()
    Y = 2 * np.fft.rfft(X)
    t = np.arange(1, L/2)
    remove_text(arr)
    ax1.clear()
    arr = draw_text(fig, Y)
    ax1.plot(Fs/L*t, abs(Y)[1:] / 1000)
    plt.pause(0.0001)
    # plt.clf()