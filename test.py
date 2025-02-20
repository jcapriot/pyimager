from pyimager.synthetics import spike, synlv
from pyimager.filters import butterworth_bandpass
from pyimager.plotting import wiggle
import matplotlib.pyplot as plt
import numpy as np

segy = synlv(ft=0.11, fxo=0.5, fxs=2.0, x0=0.1, z0=0.12, dvdx=0.13, dvdz=0.14, nxo=10, er=2, ls=3, smooth=4) #.to_memory()
# wiggle(segy)
# plt.show()

im_dat = [trace.data for trace in segy]
print(im_dat)
im_dat = np.hstack(im_dat)
print(im_dat)
plt.imshow(im_dat.T)

segy.to_file('test.segy')

bandpassed = butterworth_bandpass(segy).to_memory()

wiggle(bandpassed)
plt.show()

plt.imsho(im_dat)
plt.show()


print("succ")