from .segy cimport SEGYTrace, SEGY, TraceIterator
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
            (3 * ntr // 4, 3 * nt // 4),
        ][:nspk]

    dt = dt * 1_000_000 # second to micro

    cdef SEGY handle = SEGY.__new__(SEGY)
    handle.ntr = ntr
    handle.dt = <unsigned short> dt
    handle.ns = nt

    return SEGY.from_trace_iterator(_SpikeIterator(handle, nt, ntr, dt, offset, spikes))

cdef class _SpikeIterator(TraceIterator):
    cdef:
        int nt, ntr
        float dt, offset
        list spikes

    def __init__(self, SEGY handle, int nt, int ntr, float dt, float offset, list spikes):
        super().__init__(handle=handle)
        self.nt = nt
        self.ntr = ntr
        self.dt = dt
        self.offset = offset
        self.spikes = spikes

    cdef SEGYTrace next_trace(self):
        if self.i == self.ntr:
            raise StopIteration()
        data = np.zeros(self.nt, dtype=np.float32)
        for spike in self.spikes:
            ix, it = spike
            if ix == self.i:
                data[it - 1] = 1.0
        self.i += 1
        return SEGYTrace(data, self.dt, ntr=self.ntr, offset=self.offset, tracl=self.i+1)


