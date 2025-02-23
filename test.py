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
plt.show()

im_dat = [trace.data for trace in segy]
im_dat = np.vstack(im_dat)
plt.imshow(im_dat.T)
plt.show()

print(segy.n_traces)

print("bandpassing:")
bandpassed = butterworth_bandpass(segy).to_memory()
print("succeeded")

im_dat = [trace.data for trace in bandpassed]
im_dat = np.stack(im_dat)
plt.figure()
plt.imshow(im_dat.T, clim=[-1, 1], cmap='seismic')
plt.show()

plt.imshow(im_dat)
plt.show()


print("succ", flush=True)