# cython: embedsignature=True, language_level=3
# cython: linetrace=True

from ..segy cimport SEGYTrace, SEGY, BaseTraceIterator, segy, new_trace
import numpy as np

cdef class plane(BaseTraceIterator):
    cdef:
        unsigned short nt, dt
        int offset
        bint taper
        float msps
        # plane parameter holders
        float[:] dips
        int[:] cts
        int[:] cxs
        int[:] lens
        int n_planes

        int[:] tfes

    def __init__(self, unsigned short nt=64, int ntr=32, bint taper=False, float dt=0.004, int offset=400, planes=None):
        if planes is None:
            planes = [
                # (dip ms/trace, trace_extent, center_time, center_trace)
                (0, 3 * ntr//4, nt//2 - 1, ntr//2 - 1),
                (4, 3 * ntr//4, nt//2 - 1, ntr//2 - 1),
                (8, 3 * ntr//4, nt//2 - 1, ntr//2 - 1),
             ]
        elif planes == "liner":
            nt = 64
            ntr = 64
            planes = [
                # (dip ms/trace, trace_extent, center_time, center_trace)
                (0, ntr//4, nt//2 - 1, 3 * ntr//4 - 1),
                (4, ntr//4, nt//2 - 1, ntr//2 - 1),
                (8, ntr//4, nt//2 - 1, ntr//4 - 1),
             ]
        n_planes = len(planes)
        dips = np.empty(n_planes, dtype=np.float32)
        lens = np.empty(n_planes, dtype=np.int32)
        cts = np.empty(n_planes, dtype=np.int32)
        cxs = np.empty(n_planes, dtype=np.int32)
        for i, plane in enumerate(planes):
            if len(plane) != 4:
                raise TypeError("Must supply four values to describe a plane.")
            dips[i] = plane[0]
            lens[i] = plane[1]
            cts[i] = plane[2]
            cxs[i] = plane[3]
        self.dips = dips
        self.lens = lens
        self.cts = cts
        self.cxs = cxs
        self.tfes = np.zeros(n_planes, dtype=np.int32)

        self.nt = nt
        self.dt = <unsigned short> (dt * 1_000_000)
        self.msps = 1_000 * dt
        self.offset = offset
        self.taper = taper

        self.n_traces = ntr
        self.n_planes = self.dips.shape[0]

    cdef SEGYTrace next_trace(self):
        if self.i == self.n_traces:
            raise StopIteration()
        cdef:
            segy *tr = new_trace(self.nt)
            int i, tfe, itr, itless, itmore, cx_i, dt_i, len_i
            float eps, fit, dip_i

        itr = self.i

        tr.dt = self.dt
        tr.offset = self.offset
        tr.tracl = tr.tracr = self.i + 1
        tr.ntr = self.n_traces

        for i in range(self.n_planes):
            tfe = self.tfes[i]
            dip_i = self.dips[i]
            cx_i = self.cxs[i]
            ct_i = self.cts[i]
            len_i = self.lens[i]
            if (itr >= cx_i - len_i // 2) and (itr <= cx_i + len_i // 2):

                # fit is fractional sample of plane intersection
                fit = ct_i - ( cx_i - itr ) * dip_i / self.msps
                if (fit >= 0) and (fit < self.nt - 1):

                    # linear interpolation
                    itless = <int> fit
                    eps = fit - itless
                    itmore = <int> (fit + 1)
                    tr.data[itless] += 1.0 - eps
                    tr.data[itmore] += eps

                    # taper option
                    if self.taper:
                        # first or last point
                        if tfe == 0 or tfe == len_i:
                            tr.data[itless] /= 6.0
                            tr.data[itmore] /= 6.0
                        # second or next-to-last point
                        if tfe == 1 or tfe == len_i-1:
                            tr.data[itless] /= 3.0
                            tr.data[itmore] /= 3.0

                self.tfes[i] += 1
        self.i += 1
        return SEGYTrace.from_trace(tr, True, True)