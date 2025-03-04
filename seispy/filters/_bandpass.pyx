# cython: embedsignature=True, language_level=3
# cython: linetrace=True

from ..container cimport spy_trace, Trace, TraceCollection, BaseTraceIterator, copy_of
from ..cwp cimport bfdesign
from libc.math cimport sqrt
from libc.string cimport memcpy

cdef extern from "su_filters.h":
    void bfhighpass_trace(int zerophase, int npoles, float f3db, spy_trace *tr_in, spy_trace *tr)
    void bflowpass_trace(int zerophase, int npoles, float f3db, spy_trace *tr_in, spy_trace *tr)

cdef class butterworth_bandpass(BaseTraceIterator):
    cdef:
        int zerophase
        int low_cut, high_cut
        int npoleslo, npoleshi
        float f3dblo, f3dbhi, f3dblo_in, f3dbhi_in
        float last_dt
        BaseTraceIterator iter_in
        int design_low, design_high
        int none_f3dblo, none_f3dbhi, none_fpasslo, none_fstoplo, none_fpasshi, none_fstophi
        float fstoplo, fpasslo, fstophi, fpasshi
        float astoplo, apasslo, astophi, apasshi

    def __init__(
        self, trace_iter, bint low_cut=True, bint high_cut=True,
        f_stop_low=None, float a_stop_low=0.05, f_pass_low=None, float a_pass_low=0.95,
        f_stop_high=None, float a_stop_high=0.05, f_pass_high=None, float a_pass_high=0.95,
        int n_poles_low=0, f3db_low=None, int n_poles_high=0, f3db_high=None,
        bint zerophase=True,
    ):
        self.last_dt = 0
        if isinstance(trace_iter, TraceCollection):
            trace_iter = trace_iter.__iter__()
        self.iter_in = trace_iter
        self.n_traces = self.iter_in.n_traces
        self.zerophase = zerophase
        self.low_cut = low_cut
        self.high_cut = high_cut

        self.design_low = n_poles_low == 0
        self.design_high = n_poles_high == 0

        self.npoleslo = n_poles_low
        self.apasslo = a_pass_low
        self.astoplo = a_stop_low

        self.npoleshi = n_poles_high
        self.apasshi = a_pass_high
        self.astophi = a_stop_high

        if self.zerophase:
            self.apasslo = sqrt(self.apasslo)
            self.astoplo = sqrt(self.astoplo)
            self.apasshi = sqrt(self.apasshi)
            self.astophi = sqrt(self.astophi)

        if f3db_low is None:
            self.none_f3dblo = True
        else:
            self.f3dblo_in = f3db_low

        if f3db_high is None:
            self.none_f3dbhi = True
        else:
            self.f3dbhi_in = f3db_high

        if f_pass_low is None:
            self.none_fpasslo = True
        else:
            self.fpasslo = f_pass_low

        if f_stop_low is None:
            self.none_fstoplo = True
        else:
            self.fstoplo = f_stop_low

        if f_pass_high is None:
            self.none_fpasshi = True
        else:
            self.fpasshi = f_pass_high

        if f_stop_high is None:
            self.none_fstophi = True
        else:
            self.fstophi = f_stop_high

    cdef void set_filter_params(self, spy_trace *tr):
        cdef float fstoplo, fstophi, fpasslo, fpasshi
        if tr.hdr.d_sample != self.last_dt:
            self.last_dt = tr.hdr.d_sample

            if self.low_cut:
                if self.design_low:
                    if self.none_fstoplo:
                        fstoplo = .10 * 0.5 # nyq * dt
                    else:
                        fstoplo = self.fstoplo * tr.hdr.d_sample
                    if self.none_fpasslo:
                        fpasslo = .15 * 0.5 # nyq * dt
                    else:
                        fpasslo = self.fpasslo * tr.hdr.d_sample
                    bfdesign(fpasslo, self.apasslo, fstoplo, self.astoplo, &self.npoleslo, &self.f3dblo)
                elif self.none_f3dblo:
                    self.f3dblo = .15 * 0.5 # nyq * dt
                else:
                    self.f3dblo = self.f3dblo_in * tr.hdr.d_sample

            if self.high_cut:
                if self.design_high:
                    if self.none_fstophi:
                        fstophi = .55 * 0.5 # nyq * dt
                    else:
                        fstophi = self.fstophi * tr.hdr.d_sample
                    if self.none_fpasshi:
                        fpasshi = .40 * 0.5 # nyq * dt
                    else:
                        fpasshi = self.fpasshi * tr.hdr.d_sample
                    bfdesign(fpasshi, self.apasshi, fstophi, self.astophi, &self.npoleshi, &self.f3dbhi)
                elif self.none_f3dbhi:
                    self.f3dbhi = .40 * 0.5 # nyq * dt
                else:
                    self.f3dbhi = self.f3dbhi_in * tr.hdr.d_sample

    cdef Trace next_trace(self):
        cdef:
            Trace trace = self.iter_in.next_trace()
            spy_trace *tr = copy_of(trace.tr)
        self.set_filter_params(tr)

        if self.low_cut:
            bfhighpass_trace(self.zerophase, self.npoleslo, self.f3dblo, tr, tr)

        if self.high_cut:
            bflowpass_trace(self.zerophase, self.npoleshi, self.f3dbhi, tr, tr)

        return Trace.from_trace(tr, 1, 1)