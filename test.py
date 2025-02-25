from pyimager.segy import SEGY
from pyimager.synthetics import spike, synlv
from pyimager.filters import butterworth_bandpass
from pyimager.plotting import wiggle
import matplotlib.pyplot as plt
import numpy as np
import time
import os

segy = synlv().to_memory()
wiggle(segy)

im_dat = [trace.data for trace in segy]
im_dat = np.vstack(im_dat)

plt.figure()
plt.imshow(im_dat.T, cmap='seismic')
plt.show()

bandpassed = butterworth_bandpass(segy).to_memory()

bandpassed.to_file("test.segy")
for trace in bandpassed:
    print(np.asarray(trace.data))

im_dat = [trace.data for trace in bandpassed]
im_dat = np.stack(im_dat)
plt.figure()
plt.imshow(im_dat.T, clim=[-1, 1], cmap='seismic')
plt.show()

print("ended")