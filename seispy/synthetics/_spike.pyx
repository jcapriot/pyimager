# cython: embedsignature=True, language_level=3
# cython: linetrace=True

from ..container cimport (
    Trace, TraceCollection, BaseTraceIterator, spy_trace, spy_trace_header, new_trace, SPY_TX_GATHER
)
import numpy as np

cdef class spike(BaseTraceIterator):
    cdef:
        size_t nt
        double dt
        double offset
        # spike parameters
        int[:, ::1] spikes

    def __init__(self, size_t nt=64, size_t ntr=32, double dt=0.004, double offset=400, spikes=None):
        self.nt = nt

        self.hdr.n_traces = ntr
        self.hdr.ensemble_type = SPY_TX_GATHER
        self.hdr.uniform_traces = True
        self.dt = dt
        self.offset = offset

        if spikes is None:
            spikes = [
                 (ntr // 4, nt // 4 - 1),
                 (ntr // 4, 3 * nt // 4 - 1),
                 (3 * ntr // 4, nt // 4 - 1),
                 (3 * ntr // 4, 3 * nt // 4 - 1),
             ]

        self.spikes = np.require(spikes, dtype=np.int32, requirements='C')

    cdef Trace next_trace(self):
        if self.i == self.hdr.n_traces:
            raise StopIteration()
        cdef:
            spy_trace *tr = new_trace(self.nt)
            spy_trace_header *hdr = &(tr.hdr)
            int it, ix

        hdr.d_sample = self.dt
        hdr.tx_loc[0] = self.i
        hdr.rx_loc[0] = self.i + self.offset
        hdr.offset = self.offset
        hdr.trace_id = self.i + 1
        for spike in self.spikes:
            if spike[0] == self.i:
                tr.data[spike[1]] = 1.0
        self.i += 1
        return Trace.from_trace(tr, True, True)