from .segy cimport segy, SEGYTrace, SEGY
from .cwp cimport bfdesign, bfhighpass, bflowpass
from libc.math cimport sqrt

cdef extern from "su_filters.h":
    void bfhighpass_trace(int zerophase, int npoles, float f3db, int nt, segy *tr_data);
    void bflowpass_trace(int zerophase, int npoles, float f3db, int nt, segy *tr_data);

def butterworth_bandpass(
        SEGY input,
        low_cut=True,
        high_cut=True,
        f_stop_low=None, a_stop_low=.05, f_pass_low=None, a_pass_low=0.95,
        f_stop_high=None, a_stop_high=0.05, f_pass_high=None, a_pass_high=0.95,
        int n_poles_low=0, f3db_low=0.0, n_poles_high=0, float f3db_high=0.0,
        bint zerophase=True
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
                f_stop_high = .10 * nyq
            if f_pass_high is None:
                f_pass_high = .15 * nyq

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
                f3db_high = .15 * nyq
            f3dbhi = f3db_high * dt

    # Low-cut (high pass) filter
    def trace_iterator():
        cdef:
            SEGYTrace trace
            segy *tr
        for trace in input:
            tr = &trace.trace
            if low_cut:
                bfhighpass(npoleslo, f3dblo, tr.ns, tr.data, tr.data);
                if zerophase:
                    for i in range(tr.ns // 2): # reverse trace in place
                        tmp = tr.data[i]
                        tr.data[i] = tr.data[nt-1 - i]
                        tr.data[nt-1 - i] = tmp
                    bfhighpass(npoleslo, f3dblo, tr.ns, tr.data, tr.data)
                    for i in range(tr.ns // 2): # flip trace back
                        tmp = tr.data[i]
                        tr.data[i] = tr.data[nt-1 - i]
                        tr.data[nt-1 - i] = tmp
            if high_cut:
                bflowpass(npoleshi, f3dbhi, tr.ns, tr.data, tr.data);
                if zerophase:
                    for i in range(tr.ns // 2): # reverse trace in place
                        tmp = tr.data[i]
                        tr.data[i] = tr.data[nt-1 - i]
                        tr.data[nt-1 - i] = tmp
                    bflowpass(npoleshi, f3dbhi, tr.ns, tr.data, tr.data)
                    for i in range(tr.ns // 2): # flip trace back
                        tmp = tr.data[i]
                        tr.data[i] = tr.data[nt-1 - i]
                        tr.data[nt-1 - i] = tmp
            yield trace
    return SEGY.from_iterator(trace_iterator(), ntr=input.ntr, dt=input.dt, ns=input.ns)