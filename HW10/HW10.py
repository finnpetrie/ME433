import csv
import matplotlib.pyplot as plt
import numpy as np
import glob

t     = []
data1 = []

with open('sigA.csv', newline='') as csvfile:
    reader = csv.reader(csvfile)
    for row in reader:
        # skip empty lines
        if not row:
            continue

        # row[0] is time, row[1] is value
        t.append( float(row[0]) )
        data1.append( float(row[1]) )

sample_rate = len(data1)/(t[-1] - t[0])
# # now print them
# for i in range(len(t)):
#     print(f"{t[i]}, {data1[i]}")
plt.figure(figsize=(8, 4))
plt.plot(t, data1, marker='o', linestyle='-')
plt.xlabel('Time (s)')
plt.ylabel('Signal Amplitude')
plt.title('Signal vs. Time')
plt.grid(True)
plt.tight_layout()
plt.show()


def moving_average(x, N):
    """
    Simple moving average of window N.
    Pads the first N points with zeros so output length == input length.
    """
    padded = np.concatenate((np.zeros(N), x))
    out = np.empty_like(x)
    for i in range(len(x)):
        out[i] = padded[i+N] - padded[i]  # sum of window
        # but we need the average
    # Actually compute averages:
    out = np.convolve(x, np.ones(N)/N, mode='same')
    return out

# Try a few window sizes and plot by eye:
window_size = 30
for fname in glob.glob("*.csv"):
    # 1) Load data
    t, data = [], []
    with open(fname, newline='') as f:
        reader = csv.reader(f)
        for row in reader:
            if not row: 
                continue
            t.append(float(row[0]))
            data.append(float(row[1]))
    t    = np.array(t)
    data = np.array(data)
    
    # 2) Filter
    filt = np.convolve(data, np.ones(window_size)/window_size, mode='same')
    
    # 3) Plot
    plt.figure(figsize=(8,4))
    plt.plot(t, data, 'k', label="Raw")
    plt.plot(t, filt, 'r', label=f"MA (N={window_size})")
    plt.xlabel("Time")
    plt.ylabel("Amplitude")
    plt.title(f"{fname}   →   {window_size}-point moving average")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.show()

dt = 1.0/10000.0 # 10kHz
# t = np.arange(0.0, 1.0, dt) # 10s
# a constant plus 100Hz and 1000Hz
# s = 4.0 * np.sin(2 * np.pi * 100 * t) + 0.25 * np.sin(2 * np.pi * 1000 * t) + 25

Fs = sample_rate # sample rate
Ts = 1.0/Fs; # sampling interval
# ts = np.arange(0,t[-1],Ts) # time vector
# y = data1 # the data to make the fft from
# n = len(y) # length of the signal
# k = np.arange(n)
# T = n/Fs
# frq = k/T # two sides frequency range
# frq = frq[range(int(n/2))] # one side frequency range
# Y = np.fft.fft(y)/n # fft computing and normalization
# Y = Y[range(int(n/2))]

N = 50
filt = moving_average(np.array(data1), N)

Fs = len(data1) / (t[-1] - t[0])  # sample rate
n  = len(data1)
freq = np.fft.rfftfreq(n, d=1/Fs)

Y_raw  = np.abs(np.fft.rfft(data1) ) / n
Y_filt = np.abs(np.fft.rfft(filt) ) / n


fig, (ax1, ax2) = plt.subplots(2, 1)
ax1.plot(t,y,'b')
ax1.set_xlabel('Time')
ax1.set_ylabel('Amplitude')
ax2.loglog(frq,abs(Y),'b') # plotting the fft
ax2.set_xlabel('Freq (Hz)')
ax2.set_ylabel('|Y(freq)|')
plt.show()
