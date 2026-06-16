import serial
import numpy as np
import matplotlib.pyplot as plt
from scipy.fft import fft, fftfreq

PORT = "COM6"
BAUD = 115200
N_SAMPLES = 500

ser = serial.Serial(PORT, BAUD, timeout=10)

print("Connected")

ser.write(f"{N_SAMPLES}\n".encode())

while True:
    line = ser.readline().decode(errors='ignore').strip()

    if line == "START":
        break

time_ms = []
raw = []
filtered = []

while True:

    line = ser.readline().decode(errors='ignore').strip()

    if line == "END":
        break

    try:
        t, r, f = line.split(',')

        time_ms.append(float(t))
        raw.append(float(r))
        filtered.append(float(f))

    except:
        pass

ser.close()

time_ms = np.array(time_ms)
raw = np.array(raw)
filtered = np.array(filtered)

print(f"Received {len(raw)} samples")

# Time-domain plot

plt.figure(figsize=(10, 5))

plt.plot(time_ms, raw, label='Raw')
plt.plot(time_ms, filtered, label='Filtered')

plt.xlabel('Time (ms)')
plt.ylabel('HX711 Reading')
plt.title('HX711 Force Sensor Data')
plt.legend()
plt.grid(True)

# Estimate sample rate

dt = np.mean(np.diff(time_ms)) / 1000.0
Fs = 1.0 / dt

print(f"Sample Rate = {Fs:.2f} Hz")
print(f"Nyquist Frequency = {Fs/2:.2f} Hz")

# FFT

raw_fft = np.abs(fft(raw))
filtered_fft = np.abs(fft(filtered))

freq = fftfreq(len(raw), dt)

mask = freq >= 0

plt.figure(figsize=(10, 5))

plt.plot(freq[mask], raw_fft[mask], label='Raw FFT')
plt.plot(freq[mask], filtered_fft[mask], label='Filtered FFT')

plt.xlabel('Frequency (Hz)')
plt.ylabel('Magnitude')
plt.title('FFT of HX711 Data')
plt.xlim([0, 40])
plt.legend()
plt.grid(True)

plt.show()