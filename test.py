from pyimager.synthetics import spike
from pyimager.filters import butterworth_bandpass
import numpy as np
import matplotlib.pyplot as plt

segy = spike()
segy.to_memory()

segy.to_file('test.segy')

bandpassed = butterworth_bandpass(segy).to_memory()
for i, trace in enumerate(bandpassed):
    print(i)
    print(trace.data)


traces = np.asarray([trace.data for trace in bandpassed])
plt.imshow(traces)
plt.show()