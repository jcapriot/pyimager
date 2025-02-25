from libc.stdio cimport FILE, fwrite, fread
from libc.stdlib cimport malloc, free
cimport cpython.buffer as cpy_buf
cimport cython
from ._io cimport PyFile_Dup, PyFile_DupClose, pyi_off_t

import numpy as np

@cython.final
cdef class SEGYTrace:

    def __cinit__(self):
        self.tr = NULL
        self.trace_owner = False
        self.data_owner = False

    def __dealloc__(self):
        # De-allocate if not null and flag is set
        if self.tr is not NULL and self.trace_owner:
            del_trace(self.tr, self.data_owner)
            self.tr = NULL

    @staticmethod
    cdef SEGYTrace from_trace(segy *tr, bint trace_owner=False, bint data_owner=False):
        cdef SEGYTrace cy_trace = SEGYTrace.__new__(SEGYTrace)
        cy_trace.tr = tr
        cy_trace.trace_data = <float[:tr.ns]> tr.data
        cy_trace.trace_owner = trace_owner
        cy_trace.data_owner = data_owner
        return cy_trace

    @staticmethod
    cdef SEGYTrace from_file_descriptor(FILE *fd):

        cdef segy *tr = <segy *> malloc(sizeof(segy))
        if tr is NULL:
            raise MemoryError("Unable to allocate trace structure.")
        cdef int n_read
        n_read = fread(tr, HDRBYTES, 1, fd)
        if n_read != 1:
            del_trace(tr, 1)
            raise IOError("Unable to read trace header from file.")

        tr.data = <float *> malloc(sizeof(float)*tr.ns)
        n_read = fread(tr, sizeof(float), tr.ns, fd)
        if n_read != tr.ns:
            free(tr.data)
            raise IOError("Unable to read expected number of trace samples.")

        return SEGYTrace.from_trace(tr, True, True)

    cdef to_file_descriptor(self, FILE *fd):
        cdef segy *tr = self.tr
        cdef int n_write = fwrite(tr, HDRBYTES, 1, fd)
        if n_write != 1:
            raise IOError("Error writing trace header to file.")

        n_write = fwrite(tr.data, sizeof(float), tr.ns, fd)
        if n_write != tr.ns:
            raise IOError("Error writing trace data to file.")

    def __init__(
        self,
        data,
        unsigned short dt,
        int tracl=0,
        int tracr=0,
        int fldr=0,
        int tracf=0,
        int ep=0,
        int cdp=0,
        int cdpt=0,
        short trid=0,
        short nvs=1,
        short nhs=1,
        short duse=2,
        short offset=0,
        int gelev=0,
        int selev=0,
        int sdepth=0,
        int gdel=0,
        int sdel=0,
        int swdep=0,
        int gwdep=0,
        int scalel=1,
        int scalco=1,
        int sx=0,
        int sy=0,
        int gx=0,
        int gy=0,
        short counit=1,
        short wevel=1,
        short swevel=1,
        short sut=0,
        short gut=0,
        short sstat=0,
        short gstat=0,
        short tstat=0,
        short laga=0,
        short lagb=0,
        short delrt=0,
        short muts=-1,
        short mute=-1,
        short gain=1,
        short igc=1,
        short igi=1,
        short corr=1,
        short sfs=1,
        short sfe=120,
        short slen=10_000,
        short styp=3,
        short stas=0,
        short stae=10_000,
        short tatyp=1,
        short afilf=0,
        short afils=1,
        short nofilf=0,
        short nofils=1,
        short lcf=0,
        short hcf=0,
        short lcs=1,
        short hcs=1,
        short year=1970,
        short day=0,
        short hour=0,
        short minute=0,
        short sec=0,
        short timbas=0,
        short trwf=0,
        short grnors=0,
        short grnofr=0,
        short grnlof=0,
        short gaps=0,
        short otrav=0,
        float d1=1,
        float f1=0,
        float d2=1,
        float f2=0,
        float ungpow=0,
        float unscale=1,
        int ntr=1,
        short mark=0,
        short shortpad=0,
    ):
        self.trace_data = np.require(data, dtype=np.float32, requirements='C')
        cdef segy *tr = <segy *> malloc(sizeof(segy))
        if tr is NULL:
            raise MemoryError("Unable to allocate trace.")

        tr.tracl = tracl
        tr.tracr = tracr
        tr.fldr = fldr
        tr.tracf = tracf
        tr.ep = ep
        tr.cdp = cdp
        tr.cdpt = cdpt
        tr.trid = trid
        tr.nvs = nvs
        tr.nhs = nhs
        tr.duse = duse
        tr.offset = offset
        tr.gelev = gelev
        tr.selev = selev
        tr.sdepth = sdepth
        tr.gdel = gdel
        tr.sdel = sdel
        tr.swdep = swdep
        tr.gwdep = gwdep
        tr.scalel = scalel
        tr.scalco = scalco
        tr.sx = sx
        tr.sy = sy
        tr.gx = gx
        tr.gy = gy
        tr.counit = counit
        tr.wevel = wevel
        tr.swevel = swevel
        tr.sut = sut
        tr.gut = gut
        tr.sstat = sstat
        tr.gstat = gstat
        tr.tstat = tstat
        tr.laga = laga
        tr.lagb = lagb
        tr.delrt = delrt
        tr.muts = muts
        tr.mute = mute
        tr.ns = self.trace_data.shape[0]
        tr.dt = dt
        tr.gain = gain
        tr.igc = igc
        tr.igi = igi
        tr.corr = corr
        tr.sfs = sfs
        tr.sfe = sfe
        tr.slen = slen
        tr.styp = styp
        tr.stas = stas
        tr.stae = stae
        tr.tatyp = tatyp
        tr.afilf = afilf
        tr.afils = afils
        tr.nofilf = nofilf
        tr.nofils = nofils
        tr.lcf = lcf
        tr.hcf = hcf
        tr.lcs = lcs
        tr.hcs = hcs
        tr.year = year
        tr.day = day
        tr.hour = hour
        tr.minute = minute
        tr.sec = sec
        tr.timbas = timbas
        tr.trwf = trwf
        tr.grnors = grnors
        tr.grnofr = grnofr
        tr.grnlof = grnlof
        tr.gaps = gaps
        tr.otrav = otrav
        tr.d1 = d1
        tr.f1 = f1
        tr.d2 = d2
        tr.f2 = f2
        tr.ungpow = ungpow
        tr.unscale = unscale
        tr.ntr = ntr
        tr.mark = mark
        tr.shortpad = shortpad

        tr.data = &self.trace_data[0]

        self.tr = tr
        self.trace_owner = True
        self.trace_data_owner = False

    @property
    def ntr(self):
        return self.tr.ntr

    @property
    def ns(self):
        return self.tr.ns

    @property
    def dt(self):
        return self.tr.dt

    @property
    def data(self):
        return self.trace_data

    def __getbuffer__(self, Py_buffer *buffer, int flags):
        cdef Py_ssize_t itemsize = sizeof(self.tr.data[0])
        buffer.buf = <char *> self.tr.data
        buffer.format = 'f'
        buffer.internal = NULL
        buffer.itemsize = itemsize
        buffer.len = self.tr.ns
        buffer.ndim = 1
        buffer.obj = self
        buffer.readonly = 0
        buffer.shape = self.trace_data.shape
        buffer.strides = self.trace_data.strides
        buffer.suboffsets = NULL

cdef class SEGY:
    def __cinit__(self):
        self.file = None
        self.traces = None
        self.iterator = None
        self.fd = NULL
        self.orig_pos = 0
        self.ntr = 0

    def __dealloc__(self):
        # Ensure the file is closed on deletion
        self._close_file()

    cdef _close_file(self):
        if self.fd is not NULL:
            # close the dupped handle
            PyFile_DupClose(self.file, self.fd, self.orig_pos)
            self.fd = NULL
        if self.file_owner and self.file is not None:
            self.file.close()
            self.file_owner = False


    def __init__(self, trace_data, dt, **kwargs):
        n_tr = len(trace_data)
        traces = []
        for trace in trace_data:
            traces.append(SEGYTrace(trace, dt=dt, ntr=n_tr, **kwargs))
        self.traces = traces
        self.ntr = len(traces)

    @property
    def n_trace(self):
        return self.ntr

    @classmethod
    def from_file(cls, filename):
        if isinstance(filename, (bytes, str)):
            file = open(filename)
            file_owner = True
        else:
            file = filename
            file_owner = False

        cdef SEGY new_segy = SEGY.__new__(SEGY)

        new_segy.file = file
        fd = PyFile_Dup(file, 'rb', &new_segy.orig_pos)
        fread(&new_segy.ntr, sizeof(new_segy.ntr), 1, fd)

        return new_segy

    @staticmethod
    cdef SEGY from_trace_iterator(BaseTraceIterator iterator):
        cdef SEGY new_segy = SEGY.__new__(SEGY)
        new_segy.iterator = iterator
        new_segy.ntr = iterator.n_traces
        return new_segy

    @property
    def on_disk(self):
        return self.file is not None

    @property
    def is_iterator(self):
        return self.iterator is not None

    @property
    def in_memory(self):
        return self.traces is not None

    @property
    def n_traces(self):
        return self.ntr

    def __iter__(self):
        cdef _FileTraceIterator f_iter
        if self.on_disk:
            if self.file.seekable():
                self.file.seek(self.orig_pos)
                fread(&self.ntr, sizeof(self.ntr), 1, self.fd)
            return _FileTraceIterator.from_file_descriptor(self.fd, self.ntr)
        elif self.in_memory:
            return _MemoryTraceIterator(self.traces)
        elif self.is_iterator:
            return self.iterator
        else:
            raise TypeError('Undefined')

    def to_memory(self):
        if self.in_memory:
            return self
        else:
            self.traces = [trace for trace in self]
            self.iterator = None
            self._close_file()
            return self

    def to_file(self, filename):
        if self.on_disk:
            return self
        cdef:
            SEGYTrace trace
            FILE *fd
            bint file_owner
            pyi_off_t orig_pos = 0

        if isinstance(filename, (bytes, str)):
            file = open(filename)
            file_owner = True
        else:
            file = filename
            file_owner = False

        fd = PyFile_Dup(file, 'wb', &orig_pos)
        fwrite(&self.ntr, sizeof(self.ntr), 1, fd)
        try:
            for trace in self:
                trace.to_file_descriptor(fd)
        finally:
            if file.seekable():
                # rewind the input stream
                file.seek(orig_pos)

        self.file = filename
        self.file_owner = file_owner
        self.iterator = None
        self.traces = None
        return self


cdef class BaseTraceIterator:
    def __cinit__(self):
        self.i = 0
        self.n_traces = 0

    cdef SEGYTrace next_trace(self):
        raise NotImplementedError(f"cdef next_trace is not implemented on {type(self)}.")

    def __next__(self):
        return self.next_trace()


cdef class _MemoryTraceIterator(BaseTraceIterator):
    cdef:
        list traces

    def __init__(self, list traces):
        self.traces = traces
        # iterate through to ensure they are all SEGYTraces
        for trace in self.traces:
            if not isinstance(trace, SEGYTrace):
                raise TypeError(f"Every item in trace list must be a SEGYTrace, not a {type(trace)}")

        self.n_traces = len(traces)

    cdef SEGYTrace next_trace(self):
        if self.i == self.n_traces:
            raise StopIteration()
        cdef SEGYTrace out = self.traces[self.i]
        self.i += 1
        return out


cdef class _FileTraceIterator(BaseTraceIterator):
    cdef:
        FILE *fd

    @staticmethod
    cdef _FileTraceIterator from_file_descriptor(FILE *fd, int n_traces):
        cdef _FileTraceIterator iterator = _FileTraceIterator.__new__(_FileTraceIterator)
        iterator.fd = fd
        iterator.n_traces = n_traces

    cdef SEGYTrace next_trace(self):
        if self.i == self.n_traces:
            raise StopIteration()
        cdef SEGYTrace out = SEGYTrace.from_file_descriptor(self.fd)
        self.i += 1
        return out