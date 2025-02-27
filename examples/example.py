from pyimager.synthetics import spike, synlv
from pyimager.filters import butterworth_bandpass
from pyimager.plotting import wiggle
import matplotlib.pyplot as plt
import numpy as np


segy = synlv().to_memory()
wiggle(segy)
plt.show()

bandpassed = butterworth_bandpass(segy).to_file("bandpassed.segy")

im_dat = np.array([trace for trace in bandpassed])

plt.figure()
plt.imshow(im_dat.T, clim=[-1, 1], cmap='seismic')
plt.show()