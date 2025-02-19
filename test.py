from pyimager.synthetics import spike
from pyimager.filters import butterworth_bandpass

segy = spike()
segy.to_file('test.segy')

print("Spiked")
for trace in segy:
    print(trace.data)

bandpassed = butterworth_bandpass(segy)
bandpassed.to_file('bp.segy')

print("Bandpassed")
for trace in bandpassed:
    print(trace.data)