from .segy cimport segy, SEGYTrace, SEGY, BaseTraceIterator
from .cwp cimport bfdesign, bfhighpass, bflowpass
from libc.math cimport sqrt

cdef extern from "su_filters.h":
    void bfhighpass_trace(int zerophase, int npoles, float f3db, segy *tr)
    void bflowpass_trace(int zerophase, int npoles, float f3db, segy *tr)

def butterworth_bandpass(
        SEGY input,
        low_cut=True,
        high_cut=True,
        f_stop_low=None, a_stop_low=0.05, f_pass_low=None, a_pass_low=0.95,
        f_stop_high=None, a_stop_high=0.05, f_pass_high=None, a_pass_high=0.95,
        int n_poles_low=0, f3db_low=None, n_poles_high=0, f3db_high=None,
        int zerophase=True,
):
    cdef:
        float dt = input.dt
        int nt = input.ns
        float nyq = 0.5 / dt
        float fstoplo, fpasslo, fstophi, fpasshi
        float astoplo, apasslo, astophi, apasshi

        float f3dblo, f3dbhi
        int npoleslo, npoleshi

    if low_cut:
        if n_poles_low == 0:
            if f_stop_low is None:
                f_stop_low = .10 * nyq
            if f_pass_low is None:
                f_pass_low = .15 * nyq

            fstoplo = f_stop_low * dt
            fpasslo = f_pass_low * dt
            astoplo = a_stop_low
            apasslo = a_pass_low
            if zerophase:
                astoplo = sqrt(astoplo)
                apasslo = sqrt(apasslo)
            bfdesign(fpasslo, apasslo, fstoplo, astoplo, &npoleslo, &f3dblo)
        else:
            npoleslo = n_poles_low
            if f3db_low is None:
                f3db_low = .15 * nyq
            f3dblo = f3db_low * dt

    if high_cut:
        if n_poles_high == 0:
            if f_stop_high is None:
                f_stop_high = .55 * nyq
            if f_pass_high is None:
                f_pass_high = .40 * nyq

            fstophi = f_stop_high * dt
            fpasshi = f_pass_high * dt
            astophi = a_stop_high
            apasshi = a_pass_high
            if zerophase:
                astophi = sqrt(astophi)
                apasshi = sqrt(apasshi)
            bfdesign(fpasshi, apasshi, fstophi, astophi, &npoleshi, &f3dbhi)
        else:
            npoleshi = n_poles_high
            if f3db_high is None:
                f3db_high = .40 * nyq
            f3dbhi = f3db_high * dt

    iterator = _ButterworthBandpassIter(
        input.__iter__(), zerophase, low_cut, high_cut, npoleslo, f3dblo, npoleshi, f3dbhi
    )
    return SEGY.from_trace_iterator(iterator)

cdef class _ButterworthBandpassIter(BaseTraceIterator):
    cdef:
        int zerophase
        int low_cut, high_cut
        int npoleslo, npoleshi
        float f3dblo, f3dbhi
        BaseTraceIterator iter_in

    def __init__(
        self, BaseTraceIterator iter, int zerophase, int low_cut, int high_cut, int npoleslo, float f3dblo, int npoleshi, float f3dbhi,
    ):
        self.iter_in = iter
        self.n_traces = iter.n_traces
        self.zerophase = zerophase
        self.low_cut = low_cut
        self.high_cut = high_cut
        self.npoleslo = npoleslo
        self.npoleshi = npoleshi
        self.f3dblo = f3dblo
        self.f3dbhi = f3dbhi

    cdef SEGYTrace next_trace(self):

        cdef SEGYTrace trace = self.iter_in.next_trace()
        cdef segy *tr = trace.tr

        cdef int nt = tr.ns
        if self.low_cut:
            # print(self.zerophase, self.npoleslo, self.f3dblo, trace.trace.data)
            # bfhighpass_trace(self.zerophase, self.npoleslo, 0.061300963163375854, &trace.trace)

            bfhighpass(self.npoleslo, self.f3dblo, nt, tr.data, tr.data)
            if self.zerophase:
                for i in range(nt // 2): # reverse trace in place
                    tmp = tr.data[i]
                    tr.data[i] = tr.data[nt-1 - i]
                    tr.data[nt-1 - i] = tmp
                bfhighpass(self.npoleslo, self.f3dblo, nt, tr.data, tr.data)
                for i in range(nt // 2): # flip trace back
                    tmp = tr.data[i]
                    tr.data[i] = tr.data[nt-1 - i]
                    tr.data[nt-1 - i] = tmp

        if self.high_cut:
            # bflowpass_trace(self.zerophase, self.npoleshi, self.f3dbhi, &trace.trace)

            bflowpass(self.npoleshi, self.f3dbhi, nt, tr.data, tr.data)
            if self.zerophase:
                for i in range(nt // 2): # reverse trace in place
                    tmp = tr.data[i]
                    tr.data[i] = tr.data[nt-1 - i]
                    tr.data[nt-1 - i] = tmp
                bflowpass(self.npoleshi, self.f3dbhi, nt, tr.data, tr.data)
                for i in range(nt // 2): # flip trace back
                    tmp = tr.data[i]
                    tr.data[i] = tr.data[nt-1 - i]
                    tr.data[nt-1 - i] = tmp

        # all this is done in-place on the object returned by the iter.next_trace()
        # so just return it.
        return trace