import glob
import csv
import numpy as np
import matplotlib.pyplot as plt
from scipy.signal import firwin, freqz

numtaps_list = [31, 61, 101]        
fc_list      = [50]  # cutoff frequencies in Hz
window_list  = ['hamming', 'blackman', 'hann']   # window types

for fname in glob.glob("*.csv"):
    t, data = [], []
    with open(fname, newline='') as f:
        reader = csv.reader(f)
        for row in reader:
            if not row: continue
            t.append(float(row[0])); data.append(float(row[1]))
    t    = np.array(t)
    data = np.array(data)
    Fs   = len(data) / (t[-1] - t[0])     # sampling rate
    n    = len(data)
    freq = np.fft.rfftfreq(n, d=1/Fs)
    Y_raw = np.abs(np.fft.rfft(data)) / n

    for numtaps in numtaps_list:
      for fc in fc_list:
        for win in window_list:
         
          taps = firwin(
            numtaps=numtaps,
            cutoff=fc,
            window=win,
            fs=Fs
          )

        
          data_filt = np.convolve(data, taps, mode='same')

          Y_filt = np.abs(np.fft.rfft(data_filt)) / n

          plt.figure(figsize=(10,4))
          plt.plot(t, data,     'k', alpha=0.5, label='Raw')
          plt.plot(t, data_filt,'r',           label='Filtered')
          plt.xlabel('Time (s)')
          plt.ylabel('Amplitude')
          plt.title(
            f"{fname}  —  FIR: N={numtaps}, fc={fc} Hz, win={win}"
          )
          plt.legend()
          plt.grid(True)
          plt.tight_layout()
          plt.show()

          # 7) PLOT FFTs
          plt.figure(figsize=(8,4))
          plt.plot(freq, Y_raw,  'k', label='Raw FFT')
          plt.plot(freq, Y_filt, 'r', label='Filtered FFT')
          plt.xlabel('Frequency (Hz)')
          plt.ylabel('Magnitude')
          plt.title(
            f"{fname}  —  FFT: N={numtaps}, fc={fc} Hz, win={win}"
          )
          plt.legend()
          plt.grid(True)
          plt.tight_layout()
          plt.show()
