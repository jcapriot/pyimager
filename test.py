from pyimager.synthetics import spike, synlv
from pyimager.filters import butterworth_bandpass
from pyimager.plotting import wiggle
import matplotlib.pyplot as plt
import numpy as np

segy = synlv().to_memory()
wiggle(segy)
plt.show()

im_dat = [trace.data for trace in segy]
im_dat = np.vstack(im_dat)
plt.imshow(im_dat.T)
plt.show()

segy.to_file('test.segy')

bandpassed = butterworth_bandpass(segy).to_memory()

wiggle(bandpassed)
plt.show()

plt.imshow(im_dat)
plt.show()


print("succ", flush=True)