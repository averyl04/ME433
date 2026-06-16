import csv
import numpy as np
import matplotlib.pyplot as plt


def load_csv(filename):
    time = []
    signal = []

    with open(filename, 'r') as f:
        reader = csv.reader(f)

        for row in reader:
            try:
                time.append(float(row[0]))
                signal.append(float(row[1]))
            except:
                # Skip header row if present
                pass

    return np.array(time), np.array(signal)


def sample_rate(time):
    total_time = time[-1] - time[0]
    return len(time) / total_time



def compute_fft(signal, fs):

    N = len(signal)

    fft_vals = np.fft.rfft(signal)
    fft_mag = np.abs(fft_vals) / N

    freqs = np.fft.rfftfreq(N, d=1/fs)

    return freqs, fft_mag


def moving_average(signal, window):

    filtered = []

    for i in range(len(signal)):

        if i < window:
            filtered.append(np.mean(signal[:i+1]))
        else:
            filtered.append(np.mean(signal[i-window+1:i+1]))

    return np.array(filtered)


def iir_filter(signal, A, B):

    filtered = np.zeros(len(signal))

    filtered[0] = signal[0]

    for i in range(1, len(signal)):
        filtered[i] = A * filtered[i-1] + B * signal[i]

    return filtered


def plot_signal_and_fft(time,
                        signal,
                        filtered,
                        title):

    fs = sample_rate(time)

    freq_raw, fft_raw = compute_fft(signal, fs)
    freq_filt, fft_filt = compute_fft(filtered, fs)

    plt.figure(figsize=(12, 8))

    # Time domain
    plt.subplot(2, 1, 1)
    plt.plot(time, signal, 'k', label='Original')
    plt.plot(time, filtered, 'r', label='Filtered')

    plt.title(title)
    plt.xlabel('Time (s)')
    plt.ylabel('Amplitude')
    plt.legend()
    plt.grid(True)

    # Frequency domain
    plt.subplot(2, 1, 2)
    plt.plot(freq_raw, fft_raw, 'k', label='Original FFT')
    plt.plot(freq_filt, fft_filt, 'r', label='Filtered FFT')

    plt.xlabel('Frequency (Hz)')
    plt.ylabel('Magnitude')
    plt.legend()
    plt.grid(True)

    plt.tight_layout()
    plt.show()


files = [
    'sigA.csv',
    'sigB.csv',
    'sigC.csv',
    'sigD.csv'
]

for filename in files:

    print("\n========================")
    print(filename)
    print("========================")

    time, signal = load_csv(filename)

    fs = sample_rate(time)

    print(f"Sample Rate = {fs:.2f} Hz")

    # Moving Average Filter
    # Choose X by eye
 
    X = 10

    ma_filtered = moving_average(signal, X)

    plot_signal_and_fft(
        time,
        signal,
        ma_filtered,
        f'{filename} Moving Average (X = {X})'
    )


    # IIR Filter
    # Choose A and B by eye

    A = 0.95
    B = 0.05

    iir_filtered = iir_filter(signal, A, B)

    plot_signal_and_fft(
        time,
        signal,
        iir_filtered,
        f'{filename} IIR Filter (A={A}, B={B})'
    )