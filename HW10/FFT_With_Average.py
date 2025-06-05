import glob
import csv
import numpy as np
import matplotlib.pyplot as plt


A_list = [0.9, 0.95, 0.99]

# Loop over all CSV files in current directory
for fname in glob.glob("*.csv"):
    # Load time and signal
    data = np.loadtxt(fname, delimiter=',')
    t = data[:, 0]
    y = data[:, 1]
    
    # Sampling parameters
    Fs = len(y) / (t[-1] - t[0])
    n = len(y)
    freq = np.fft.rfftfreq(n, d=1/Fs)
    
    # Compute raw FFT
    Y_raw = np.abs(np.fft.rfft(y)) / n

    # Find best A by minimizing high-frequency energy
    best_A = None
    best_metric = np.inf
    best_Yf = None
    for A in A_list:
        B = 1.0 - A
        # IIR filter
        y_filt = np.zeros_like(y)
        y_filt[0] = y[0]
        for i in range(1, n):
            y_filt[i] = A * y_filt[i-1] + B * y[i]
        # FFT of filtered
        Yf = np.abs(np.fft.rfft(y_filt)) / n
        # Metric: mean magnitude above 30% of Nyquist
        cutoff = 0.3 * freq.max()
        metric = np.mean(Yf[freq > cutoff])
        if metric < best_metric:
            best_metric = metric
            best_A = A
            best_Yf = Yf
    
    # Plot FFT comparison
    plt.figure(figsize=(8, 4))
    plt.plot(freq, Y_raw, 'k', label='Raw FFT')
    plt.plot(freq, best_Yf, 'r', label=f'IIR Filter FFT (A={best_A})')
    plt.xlabel('Frequency (Hz)')
    plt.ylabel('Magnitude')
    plt.title(f'{fname} — Raw vs IIR-filtered FFT (A={best_A})')
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.show()