import matplotlib.pyplot as plt
import numpy as np
import math

def beautify(num):
    return f'{round(num, 3):.3f}'

def dbfft(rfft, ref):
    # win = np.ones((L - 1,))
    # rfft = rfft * win

    # Scale the magnitude of FFT by window and factor of 2,
    # because we are using half of FFT spectrum.
    s_mag = np.abs(rfft)

    # Convert to dBFS
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

fig, (ax1, ax2, ax3) = plt.subplots(1, 3)
for a in [ax1, ax2, ax3]:
    a.grid()
    a.locator_params(axis='x', nbins=20)
    a.locator_params(axis='y', nbins=20)

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
fig.set_size_inches(14, 6)
fig.savefig('test.jpg', dpi=150)
# set_box_aspect
plt.show()