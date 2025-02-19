from .segy cimport SEGYTrace, SEGY
import numpy as np

def spike(
        int nt=64,
        int ntr=32,
        float dt=0.004,
        float offset=400,
        spikes=None,
        nspk = 4,
):
    cdef:
        int ix, it
    if spikes is None:
        spikes = [
            (ntr // 4,  nt // 4),
            (ntr // 4, 3 * nt // 4),
            (3 * ntr // 4, nt // 4),
            (3 * ntr // 4, 3 * ntr // 4),
        ][:nspk]

    dt = dt * 1_000_000 # second to micro

    def trace_iterator():
        for i in range(ntr):
            data = np.zeros(nt, dtype=np.float32)
            for spike in spikes:
                ix, it = spike
                if ix == i:
                    data[it - 1] = 1.0
            yield SEGYTrace(data, dt, ntr=ntr, offset=offset, tracl=i+1)

    return SEGY.from_iterator(trace_iterator(), ntr=ntr, dt=dt, ns=nt)

