from pyimager.synthetics import spike, synlv
from pyimager.filters import butterworth_bandpass
import matplotlib.pyplot as plt
import numpy as np

segy = synlv().to_memory()

bandpassed = butterworth_bandpass(segy).to_memory()

bandpassed.to_file("test.segy")
print("written")

im_dat = [trace for trace in bandpassed]
im_dat = np.stack(im_dat)

print(bandpassed.on_disk, bandpassed.in_memory, bandpassed.is_iterator)

bandpassed.to_memory()
print(bandpassed.on_disk, bandpassed.in_memory, bandpassed.is_iterator)

plt.figure()
plt.imshow(im_dat.T, clim=[-1, 1], cmap='seismic')
plt.show()

print("ended")