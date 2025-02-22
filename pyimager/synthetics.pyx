from .segy cimport SEGYTrace, SEGY, BaseTraceIterator, segy, new_trace
from .par cimport Reflector, Wavelet, breakReflectors, makeref, makericker
import numpy as np
from libc.stdlib cimport malloc, free
from libc.float cimport FLT_MAX
from libc.math cimport sqrtf
from libc.stdio cimport printf
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

    return SEGY.from_trace_iterator(_SpikeIterator(nt, ntr, <unsigned short>dt, <unsigned short>offset, spikes))

cdef class _SpikeIterator(BaseTraceIterator):
    cdef:
        unsigned short nt, dt
        int offset
        list spikes

    def __init__(self, unsigned short nt, int ntr, unsigned short dt, int offset, list spikes):
        self.nt = nt
        self.n_traces = ntr
        self.dt = dt
        self.offset = offset
        self.spikes = spikes

    cdef SEGYTrace next_trace(self):
        if self.i == self.n_traces:
            raise StopIteration()
        cdef segy *tr = new_trace(self.nt)
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


def synlv(
        int nt=101, float dt=0.04, float ft=0.0, int kilounits=1,
        int nxo=1, float dxo=0.05, float fxo=0.0, xo=None,
        nxm=None, dxm=None, float fxm=0.0,
        int nxs=101, float dxs=0.05, float fxs=0.0,
        float x0=0.0, float z0=0.0, float v00=2.0, float dvdx=0.0, float dvdz=0.0,
        fpeak=None, list reflectors=None,
        int smooth=0, int er=0, int ls=0, int ob=1,
        tmin=None, int ndpfz=5, verbose=False,
):
    cdef:
        int nr, ir, ixz, ns
        int ixo, ixsm, nxsm, tracl
        int shots, midpoints
        int *nxz

        int nxm_c
        float dxm_c = 1.5

        float vmin, tminr, tmin_c
        float x, z, v, t, dsmax, fpeak_c, dxsm
        float xs, zs, xg, zg
        float[::1] xo_c
        float *ar
        float **xr
        float **zr
        Reflector* r
        Wavelet* w

    if tmin is None:
        tmin = 10.0 * dt
    if fpeak is None:
        fpeak = 0.2 / dt

    tmin_c = tmin
    fpeak_c = fpeak

    shots = bool(nxs or dxs or fxs)
    midpoints = bool(nxm or dxm or fxm)
    if shots and midpoints:
        raise TypeError("Cannot specify both shot and midpoint sampling!")

    if shots:
        nxsm = nxs
        dxsm = dxs
    elif midpoints:
        if nxm is None:
            nxm = 101
        nxm_c = nxm
        if dxm is None:
            dxm = 0.05
        dxm_c = dxm

        nxsm = nxm_c
        dxsm = dxm_c
    else:
        raise TypeError("Must specify one of short or midpoint samplings!")

    if xo is None:
        xo_c = np.empty(nxo, dtype=np.float32)
        for ixo in range(nxo):
            xo_c[ixo] = fxo + ixo * dxo
    else:
        xo_c = np.require(xo, dtype=np.float32, flags='C')

    if reflectors is None:
        reflectors = [[1, (1, 4), (2, 2)]]
    nr = len(reflectors)
    ar = <float *> malloc(sizeof(float) * nr)
    xr = <float **> malloc(sizeof(float*) * nr)
    zr = <float **> malloc(sizeof(float*) * nr)
    nxz = <int *> malloc(sizeof(int)* nr)
    for ir, ref in enumerate(reflectors):
        if not isinstance(ref[0], tuple):
            ar[ir] = ref[0]
            n_segments = len(ref) - 1
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
        breakReflectors(&nr,&ar,&nxz,&xr,&zr)

    v00 -= dvdx * x0 + dvdz * z0

	# determine minimum velocity and minimum reflection time
    vmin = tminr = FLT_MAX
    for ir in range(nr):
        for ixz in range(nxz[ir]):
            x = xr[ir][ixz]
            z = zr[ir][ixz]
            v = v00 + dvdx * x + dvdz * z
            if v<vmin: vmin = v
            t = 2.0 * z / v
            if t<tmin_c: tminr = t

    # determine maximum reflector segment length
    tmin_c = max(tmin_c,max(ft,dt))
    dsmax = vmin/(2 * ndpfz) * sqrtf(tmin_c/fpeak_c)


    # will deallocate ar, nxz, xr, and zr
    # and allocate r
    makeref(dsmax,nr,ar,nxz,xr,zr,&r)

    # will allocate w
    makericker(fpeak_c, dt, &w)

    cdef _synlvIterator iter = _synlvIterator(
            shots, kilounits, ls, er, ob,
            fxs, dxs, nxsm,
            xo_c, dxo, fxo,
            dxm_c, fxm, dxsm,
            v00, dvdx, dvdz,
            nr, ft,
            dt, nt
    )
    iter.r = r
    iter.w = w

    return SEGY.from_trace_iterator(iter)


cdef class _synlvIterator(BaseTraceIterator):
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
            self, int shots, int kilounits, int ls, int er, int ob,
            float fxs, float dxs, int nxsm,
            float[::1] xo, float dxo, float fxo,
            float dxm, float fxm, float dxsm,
            float v00, float dvdx, float dvdz,
            int nr, float ft,
            float dt, int ns

         ):

        #init iterators
        self.ixo = 0
        self.ixsm = 0
        self.tracl = 0

        # options:
        self.shots = shots
        self.kilounits = kilounits
        self.ls = ls
        self.er = er
        self.ob = ob

        # parameters
        self.fxs = fxs
        self.dxs = dxs
        self.nxsm = nxsm

        self.xo = xo
        self.dxo = dxo
        self.fxo = fxo
        self.nxo = xo.shape[0]

        self.dxm = dxm
        self.fxm = fxm
        self.dxsm = dxsm

        self.v00 = v00
        self.dvdx = dvdx
        self.dvdz = dvdz
        self.nr = nr
        self.ft = ft

        self.dt = dt
        self.ns = ns
        self.n_traces = self.nxo * self.nxsm

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