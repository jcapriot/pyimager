from seispy.synthetics import spike, synlv
from seispy.filters import butterworth_bandpass
from seispy.plotting import wiggle
import matplotlib.pyplot as plt
import numpy as np


segy = synlv().to_memory()
wiggle(segy)
plt.show()

bandpassed = butterworth_bandpass(segy)

im_dat = np.array([trace for trace in bandpassed])

plt.figure()
plt.imshow(im_dat.T, clim=[-1, 1], cmap='seismic')
plt.show()