# cython: embedsignature=True, language_level=3
# cython: linetrace=True

from ..segy cimport SEGYTrace, SEGY, BaseTraceIterator, segy, new_trace
import numpy as np

cdef class spike(BaseTraceIterator):
    cdef:
        unsigned short nt, dt
        int offset
        # spike parameters
        int[:, ::1] spikes

    def __init__(self, unsigned short nt=64, int ntr=32, float dt=0.004, int offset=400, spikes=None):
        self.nt = nt
        self.n_traces = ntr
        self.dt = <unsigned short > dt * 1_000_000
        self.offset = offset

        if spikes is None:
            spikes = [
                 (ntr // 4, nt // 4 - 1),
                 (ntr // 4, 3 * nt // 4 - 1),
                 (3 * ntr // 4, nt // 4 - 1),
                 (3 * ntr // 4, 3 * nt // 4 - 1),
             ]

        self.spikes = np.require(spikes, dtype=np.int32, requirements='C')

    cdef SEGYTrace next_trace(self):
        if self.i == self.n_traces:
            raise StopIteration()
        cdef:
            segy *tr = new_trace(self.nt)
            int it, ix

        tr.dt = self.dt
        tr.offset = self.offset
        tr.tracl = self.i + 1
        tr.ntr = self.n_traces
        for spike in self.spikes:
            if spike[0] == self.i:
                tr.data[spike[it]] = 1.0
        self.i += 1
        return SEGYTrace.from_trace(tr, True, True)