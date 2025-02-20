from pyimager.synthetics import spike
from pyimager.filters import butterworth_bandpass
from pyimager.plotting import wiggle
import matplotlib.pyplot as plt

segy = spike().to_memory()
wiggle(segy)
plt.show()

segy.to_file('test.segy')

bandpassed = butterworth_bandpass(spike(nt=1000)).to_memory()

wiggle(bandpassed)
plt.show()