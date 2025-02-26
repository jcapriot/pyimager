from .segy cimport segy, SEGYTrace, SEGY, BaseTraceIterator
from .cwp cimport bfdesign, bfhighpass, bflowpass
from libc.math cimport sqrt

cdef extern from "su_filters.h":
    void bfhighpass_trace(int zerophase, int npoles, float f3db, segy *tr_in, segy *tr)
    void bflowpass_trace(int zerophase, int npoles, float f3db, segy *tr_in, segy *tr)

def butterworth_bandpass(
        SEGY input,
        low_cut=True,
        high_cut=True,
        f_stop_low=None, float a_stop_low=0.05, f_pass_low=None, float a_pass_low=0.95,
        f_stop_high=None, float a_stop_high=0.05, f_pass_high=None, float a_pass_high=0.95,
        int n_poles_low=0, f3db_low=None, int n_poles_high=0, f3db_high=None,
        int zerophase=True,
):

    iterator = _ButterworthBandpassIter(
        input.__iter__(), zerophase,
        low_cut, n_poles_low, f3db_low, f_pass_low, a_pass_low, f_stop_low, a_stop_low,
        high_cut, n_poles_high, f3db_high, f_pass_high, a_pass_high, f_stop_high, a_stop_high
    )
    return SEGY.from_trace_iterator(iterator)


cdef class _ButterworthBandpassIter(BaseTraceIterator):
    cdef:
        int zerophase
        int low_cut, high_cut
        int npoleslo, npoleshi
        float f3dblo, f3dbhi, f3dblo_in, f3dbhi_in
        unsigned short last_dt
        BaseTraceIterator iter_in
        int design_low, design_high
        int none_f3dblo, none_f3dbhi, none_fpasslo, none_fstoplo, none_fpasshi, none_fstophi
        float fstoplo, fpasslo, fstophi, fpasshi
        float astoplo, apasslo, astophi, apasshi

    def __init__(
        self, BaseTraceIterator iter, int zerophase,
        int low_cut, int npoleslo, f3dblo, fpasslo, float apasslo, fstoplo, float astoplo,
        int high_cut, int npoleshi, f3dbhi, fpasshi, float apasshi, fstophi, float astophi,
    ):
        self.last_dt = 0
        self.iter_in = iter
        self.n_traces = iter.n_traces
        self.zerophase = zerophase
        self.low_cut = low_cut
        self.high_cut = high_cut

        self.design_low = npoleslo == 0
        self.design_high = npoleshi == 0

        self.npoleslo = npoleslo
        self.apasslo = apasslo
        self.astoplo = astoplo

        self.npoleshi = npoleshi
        self.apasshi = apasshi
        self.astophi = astophi

        if self.zerophase:
            self.apasslo = sqrt(self.apasslo)
            self.astoplo = sqrt(self.astoplo)
            self.apasshi = sqrt(self.apasshi)
            self.astophi = sqrt(self.astophi)

        if f3dblo is None:
            self.none_f3dblo = True
        else:
            self.f3dblo_in = f3dblo

        if f3dbhi is None:
            self.none_f3dbhi = True
        else:
            self.f3dbhi_in = f3dbhi

        if fpasslo is None:
            self.none_fpasslo = True
        else:
            self.fpasslo = fpasslo

        if fstoplo is None:
            self.none_fstoplo = True
        else:
            self.fstoplo = fstoplo

        if fpasshi is None:
            self.none_fpasshi = True
        else:
            self.fpasshi = fpasshi

        if fstophi is None:
            self.none_fstophi = True
        else:
            self.fstophi = fstophi

    cdef set_filter_params(self, segy *tr):
        cdef float dt
        cdef float fstoplo, fstophi, fpasslo, fpasshi
        if tr.dt != self.last_dt:
            self.last_dt = tr.dt
            dt = <float>((<double> tr.dt)/1000000.0)

            if self.low_cut:
                if self.design_low:
                    if self.none_fstoplo:
                        fstoplo = .10 * 0.5 # nyq * dt
                    else:
                        fstoplo = self.fstoplo * dt
                    if self.none_fpasslo:
                        fpasslo = .15 * 0.5 # nyq * dt
                    else:
                        fpasslo = self.fpasslo * dt
                    bfdesign(fpasslo, self.apasslo, fstoplo, self.astoplo, &self.npoleslo, &self.f3dblo)
                elif self.none_f3dblo:
                    self.f3dblo = .15 * 0.5 # nyq * dt
                else:
                    self.f3dblo = self.f3dblo_in * dt

            if self.high_cut:
                if self.design_high:
                    if self.none_fstophi:
                        fstophi = .55 * 0.5 # nyq * dt
                    else:
                        fstophi = self.fstophi * dt
                    if self.none_fpasshi:
                        fpasshi = .40 * 0.5 # nyq * dt
                    else:
                        fpasshi = self.fpasshi * dt
                    bfdesign(fpasshi, self.apasshi, fstophi, self.astophi, &self.npoleshi, &self.f3dbhi)
                elif self.none_f3dbhi:
                    self.f3dbhi = .40 * 0.5 # nyq * dt
                else:
                    self.f3dbhi = self.f3dbhi_in * dt

    cdef SEGYTrace next_trace(self):
        cdef SEGYTrace trace = self.iter_in.next_trace()
        cdef segy *tr = trace.tr
        self.set_filter_params(tr)
        if self.low_cut:
            bfhighpass_trace(self.zerophase, self.npoleslo, self.f3dblo, tr, tr)

        if self.high_cut:
            bflowpass_trace(self.zerophase, self.npoleshi, self.f3dbhi, tr, tr)
        return trace