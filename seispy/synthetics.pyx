# cython: embedsignature=True, language_level=3
# cython: linetrace=True

from .segy cimport SEGYTrace, SEGY, BaseTraceIterator, segy, new_trace
from .par cimport Reflector, Wavelet, breakReflectors, makeref, makericker
import numpy as np
from libc.stdlib cimport malloc, free
from libc.float cimport FLT_MAX
from libc.math cimport sqrtf
cimport cython

cdef extern from "synthetics.h" nogil:
    void susynlv_filltrace(
            segy *tr, int shots, int kilounits, int tracl,
            float fxs, int ixsm, float dxs,
            int ixo, float *xo, float dxo, float fxo,
            float dxm, float fxm, float dxsm,
            float v00, float dvdx, float dvdz, int ls, int er, int ob, Wavelet *w,
            int nr, Reflector *r, int nt, float dt, float ft
    )

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
                 (ntr // 4, nt // 4),
                 (ntr // 4, 3 * nt // 4),
                 (3 * ntr // 4, nt // 4),
                 (3 * ntr // 4, 3 * nt // 4),
             ]

        self.spikes = np.require(spikes, dtype=np.int32, flags='C')

    cdef SEGYTrace next_trace(self):
        if self.i == self.n_traces:
            raise StopIteration()
        cdef:
            segy *tr = new_trace(self.nt)
            int it, ix

        for it in range(tr.ns):
            tr.data[it] = 0.0

        tr.dt = self.dt
        tr.offset = self.offset
        tr.tracl = self.i + 1
        tr.ntr = self.n_traces
        for spike in self.spikes:
            ix, it = spike
            if ix == self.i:
                tr.data[it - 1] = 1.0
        self.i += 1
        return SEGYTrace.from_trace(tr, True, True)

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
                (0, 3 * ntr, nt//2, ntr//2),
                (4, 3 * ntr//4, nt//2, ntr//2),
                (8, 3 * ntr//4, nt//2, ntr//2),
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
        print(n_planes)
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
        self.dt = <unsigned short> dt * 1_000_000
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
        for it in range(self.nt):
            tr.data[it] = 0.0

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

                    # linear interpolation * /
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
                        # second or next-to-last point * /
                        if tfe == 1 or tfe == len_i-1:
                            tr.data[itless] /= 3.0
                            tr.data[itmore] /= 3.0

                self.tfes[i] += 1
        self.i += 1
        return SEGYTrace.from_trace(tr, True, True)


cdef class synlv(BaseTraceIterator):
    cdef:
        int shots, kilounits, ls, er, ob
        int nxsm, nr, nxo, ns

        float fxs, dxs, dxo, fxo, dxm, fxm, dxsm, v00, dvdx, dvdz, ft, dt
        float[::1] xo

        # iterator indices
        int ixsm, ixo, tracl

        Wavelet *w
        Reflector *r

    def __dealloc__(self):
        # clear the memory used by wavelet and reflector objects
        if self.w is not NULL:
            free(self.w.wv)
            free(self.w)
            self.w = NULL
        if self.r is not NULL:
            for i in range(self.nr):
                free(self.r[i].rs)
            free(self.r)
            self.r = NULL

    def __init__(
            self,
            int nt=101, float dt=0.04, float ft=0.0, bint kilounits=True,
            int nxo=1, float dxo=0.05, float fxo=0.0, xo=None,
            int nxm=101, float dxm=0.05, float fxm=0.0,
            int nxs=0, float dxs=0.05, float fxs=0.0,
            float x0=0.0, float z0=0.0, float v00=2.0, float dvdx=0.0, float dvdz=0.0,
            fpeak=None, list reflectors=None,
            bint smooth=False, bint er=False, bint ls=False, int ob=True,
            tmin=None, int ndpfz=5, bint verbose=False,
    ):

        if tmin is None:
            tmin = 10.0 * dt
        if fpeak is None:
            fpeak = 0.2 / dt

        cdef float tmin_c = tmin
        cdef float fpeak_c = fpeak

        self.ns = nt

        # options:
        self.kilounits = kilounits
        self.ls = ls
        self.er = er
        self.ob = ob

        # parameters
        self.fxs = fxs
        self.dxs = dxs
        self.dxm = dxm

        self.dxo = dxo
        self.fxo = fxo

        # self.dxm = dxm
        self.fxm = fxm
        self.ft = ft
        self.dt = dt

        self.shots = bool(nxs)
        midpoints = bool(nxm)
        if self.shots and midpoints:
            raise TypeError("Cannot specify both shot and midpoint sampling!")

        if self.shots:
            self.nxsm = nxs
            self.dxsm = dxs
        elif midpoints:
            self.nxsm = nxm
            self.dxsm = dxm
        else:
            raise TypeError("Must specify one of shot or midpoint samplings!")

        if xo is None:
            self.xo = np.empty(nxo, dtype=np.float32)
            for ixo in range(nxo):
                self.xo[ixo] = fxo + ixo * dxo
        else:
            self.xo = np.require(xo, dtype=np.float32, flags='C')
        self.nxo = self.xo.shape[0]
        self.n_traces = self.nxo * self.nxsm

        if reflectors is None:
            reflectors = [[1, (1, 4), (2, 2)]]
        # decode reflectors
        self.nr = len(reflectors)
        cdef:
            float *ar = <float *> malloc(sizeof(float) * self.nr)
            float **xr = <float **> malloc(sizeof(float*) * self.nr)
            float **zr = <float **> malloc(sizeof(float*) * self.nr)
            int *nxz = <int *> malloc(sizeof(int) * self.nr)
        for ir, ref in enumerate(reflectors):
            if not isinstance(ref[0], tuple):
                ar[ir] = ref[0]
                ref = ref[1:]
            else:
                ar[ir] = 1.0
            n_segments = len(ref)
            nxz[ir] = n_segments
            xr[ir] = <float *> malloc(sizeof(float) * n_segments)
            zr[ir] = <float *> malloc(sizeof(float) * n_segments)
            for i_s, (x, z) in enumerate(ref):
                xr[ir][i_s] = x
                zr[ir][i_s] = z
        if not smooth:
            breakReflectors(&self.nr, &ar, &nxz, &xr, &zr)

        self.dvdx = dvdx
        self.dvdz = dvdz

        self.v00 = v00 - (dvdx * x0 + dvdz * z0)

        # determine minimum velocity and minimum reflection time
        cdef:
            float vmin = FLT_MAX
            float tminr = FLT_MAX
        for ir in range(self.nr):
            for ixz in range(nxz[ir]):
                x = xr[ir][ixz]
                z = zr[ir][ixz]
                v = v00 + dvdx * x + dvdz * z
                if v<vmin: vmin = v
                t = 2.0 * z / v
                if t<tmin_c: tminr = t

        # determine maximum reflector segment length
        tmin_c = max(tmin_c,max(ft,dt))
        cdef float dsmax = vmin/(2 * ndpfz) * sqrtf(tmin_c/fpeak_c)


        # will deallocate ar, nxz, xr, and zr
        # and allocate r
        makeref(dsmax, self.nr, ar, nxz, xr, zr, &self.r)

        # will allocate w
        makericker(fpeak_c, dt, &self.w)

        #init iterators
        self.ixo = 0
        self.ixsm = 0
        self.tracl = 0

    @cython.boundscheck(False)
    cdef SEGYTrace next_trace(self):
        if self.ixsm == self.nxsm:
            raise StopIteration()

        cdef segy *tr = new_trace(self.ns)
        tr.trid = 1
        tr.counit = 1
        # tr.ns = self.ns
        tr.dt = <unsigned short> (1.0e6*self.dt)
        tr.delrt = <short> (1.0e3*self.ft)
        tr.ntr = self.n_traces

        susynlv_filltrace(
            tr,
            self.shots, self.kilounits, self.tracl,
            self.fxs, self.ixsm, self.dxs,
            self.ixo, &self.xo[0], self.dxo, self.fxo,
            self.dxm, self.fxm, self.dxsm,
            self.v00, self.dvdx, self.dvdz,
            self.ls, self.er, self.ob,
            self.w, self.nr, self.r,
            self.ns, self.dt, self.ft,
        )

        # post update iters
        self.ixo += 1
        self.tracl += 1
        if self.ixo == self.nxo:
            self.ixo = 0
            self.ixsm += 1
        return SEGYTrace.from_trace(tr, True, True)