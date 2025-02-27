# cython: embedsignature=True, language_level=3
# cython: linetrace=True
import io

from cpython.bytes cimport PyBytes_FromStringAndSize
from libc.stdio cimport FILE, fwrite, fread, SEEK_CUR
from libc.stdlib cimport malloc, free
from libc.string cimport memset, memcpy
cimport cython
cimport cpython.buffer as pybuf

from ._io cimport PyFile_Dup, PyFile_DupClose, pyi_off_t, pyi_fseek

import os
from contextlib import nullcontext
import numpy as np

cdef segy* new_trace(int ns) nogil:
    cdef segy *tr = <segy *> malloc(sizeof(segy))
    memset(tr, 0, HDRBYTES)
    if ns > 0:
        tr.ns = ns
        tr.data = <float *> malloc(sizeof(float) * tr.ns)
    else:
        tr.data = NULL
    return tr

cdef segy* copy_of(segy *tr_in, bint copy_data=True) nogil:
    cdef segy *tr_copy = <segy *> malloc(sizeof(segy))
    memcpy(tr_copy, tr_in, HDRBYTES)
    if tr_in.ns > 0:
        tr_copy.data = <float *> malloc(sizeof(float) * tr_in.ns)
        memcpy(tr_copy.data,  tr_in.data, sizeof(float) * tr_in.ns)
    else:
        tr_copy.data = NULL
    return tr_copy

cdef void del_trace(segy *tp, bint del_data) nogil:
    if (tp != NULL) and del_data:
        if tp.data != NULL:
            free(tp.data)
        tp.data = NULL
    free(tp)

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
        cdef segy *tr = new_trace(0)
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

        cdef segy *tr = new_trace(0)
        if tr is NULL:
            raise MemoryError("Unable to allocate trace structure.")
        cdef int n_read
        n_read = fread(tr, HDRBYTES, 1, fd)
        if n_read != 1:
            del_trace(tr, 1)
            raise IOError("Unable to read trace header from file.")

        tr.data = <float *> malloc(sizeof(float)*tr.ns)
        n_read = fread(tr.data, sizeof(float), tr.ns, fd)
        if n_read != tr.ns:
            free(tr.data)
            raise IOError("Unable to read expected number of trace samples.")

        return SEGYTrace.from_trace(tr, True, True)

    cdef to_file_descriptor(self, FILE *fd):
        cdef int n_write = fwrite(self.tr, HDRBYTES, 1, fd)
        if n_write != 1:
            raise IOError("Error writing trace header to file.")

        n_write = fwrite(self.tr.data, sizeof(float), self.tr.ns, fd)
        if n_write != self.tr.ns:
            raise IOError("Error writing trace data to file.")

    def __getbuffer__(self, Py_buffer *buffer, int flags):
        cdef Py_ssize_t itemsize = sizeof(float)

        buffer.obj = self
        buffer.buf = <void *> self.tr.data
        buffer.len = self.tr.ns
        buffer.itemsize = itemsize
        buffer.ndim = 1

        if (flags & pybuf.PyBUF_ND) == pybuf.PyBUF_ND:
            buffer.shape = self.trace_data.shape
        else:
            buffer.shape = NULL

        if (flags & pybuf.PyBUF_STRIDES) == pybuf.PyBUF_STRIDES:
            buffer.strides = self.trace_data.strides
        else:
            buffer.strides = NULL

        if (flags & pybuf.PyBUF_WRITABLE) == pybuf.PyBUF_WRITABLE:
            buffer.readonly = 0
        else:
            buffer.readonly = 1

        if (flags & pybuf.PyBUF_FORMAT) == pybuf.PyBUF_FORMAT:
            buffer.format = 'f'
        else:
            buffer.format = NULL

        buffer.internal = NULL
        buffer.suboffsets = NULL

    def __releasebuffer__(self, Py_buffer *buffer):
        pass

cdef class SEGY:
    def __cinit__(self):
        self.file = None
        self.traces = None
        self.iterator = None
        self.ntr = 0


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
        cdef:
            SEGYTrace trace
            FILE *fd
            bint file_owner
            pyi_off_t orig_pos = 0
            int ntr = 0

        if hasattr(filename, 'read'):
            ctx = nullcontext(filename)
        else:
            ctx = open(os.fspath(filename), "rb")

        with ctx as f:
            ntr = int.from_bytes(f.read(sizeof(ntr)), byteorder='little')

        cdef SEGY new_segy = SEGY.__new__(SEGY)

        new_segy.file = filename
        new_segy.ntr = ntr

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
        if self.on_disk:
            return _FileTraceIterator(self.file, self.ntr)
        elif self.in_memory:
            return _MemoryTraceIterator(self.traces)
        elif self.is_iterator:
            return self.iterator
        else:
            raise TypeError('SEGY file is not on disk, in memory, nor from an iterator.')

    def to_memory(self):
        if self.in_memory:
            return self
        else:
            self.traces = [trace for trace in self]
            self.iterator = None
            self.file = None
            return self

    def to_file(self, filename):
        if self.on_disk:
            return self
        cdef:
            SEGYTrace trace
            FILE *fd
            bint file_owner
            pyi_off_t orig_pos = 0


        if hasattr(filename, 'write'):
            ctx = nullcontext(filename)
            try:
                filename = filename.name
            except AttributeError:
                filename = None
            file_owner = False
        else:
            ctx = open(os.fspath(filename), "wb")
            file_owner = True

        with ctx as file:
            fd, orig_pos = PyFile_Dup(file, "wb")
            try:
                fwrite(&self.ntr, sizeof(self.ntr), 1, fd)
                for trace in self:
                    trace.to_file_descriptor(fd)
            finally:
                PyFile_DupClose(file, fd, orig_pos)
            file.flush()
            if file_owner:
                os.fsync(file.fileno())

        self.file = filename
        self.iterator = None
        self.traces = None
        return self

    def to_stream(self, stream):

        if hasattr(stream, 'write'):
            ctx = nullcontext(stream)
        else:
            ctx = open(stream, 'wb')

        cdef:
            SEGYTrace trace

        with ctx as stream_ctx:
            stream_ctx.write(PyBytes_FromStringAndSize(<char *> &self.ntr, sizeof(self.ntr)))
            for trace in self:
                stream_ctx.write(PyBytes_FromStringAndSize(<char *> trace.tr, HDRBYTES))
                stream_ctx.write(PyBytes_FromStringAndSize(<char *> trace.tr.data, sizeof(float)*trace.tr.ns))
            stream_ctx.flush()
        if self.is_iterator:
            # the above will consume the iterator if it came from one.
            self.iterator = None
            # otherwise, don't do anything to the underlying object



cdef class BaseTraceIterator:
    def __cinit__(self):
        self.i = 0
        self.n_traces = 0

    cdef SEGYTrace next_trace(self):
        raise NotImplementedError(f"cdef next_trace is not implemented on {type(self)}.")

    def __next__(self):
        return self.next_trace()

    def __iter__(self):
        return self

    def to_memory(self):
        return SEGY.from_trace_iterator(self).to_memory()

    def to_file(self, filename):
        return SEGY.from_trace_iterator(self).to_file(filename)

    def to_stream(self, stream):
        SEGY.from_trace_iterator(self).to_stream(stream)


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
        bint owner
        object file
        pyi_off_t orig_pos

    def __cinit__(self):
        self.fd = NULL
        self.owner = False
        self.file = None
        self.n_traces = 0

    def __dealoc__(self):
        # make sure I get closed up when I'm garbage collected
        self._close_file()

    cdef _close_file(self):
        # first close my duped file
        if self.fd is not NULL and self.file is not None:
            PyFile_DupClose(self.file, self.fd, self.orig_pos)
            self.fd = NULL
        # If I own the original, close it
        if self.owner and self.file is not None:
            self.file.close()
        # and clear my reference to the original
        self.file = None

    def __init__(self, file, int n_traces):
        if not hasattr(file, "read"):
            # open the file
            file = open(os.fspath(file), "rb")
            self.owner = True
        else:
            self.owner = False
        self.file = file
        self.n_traces = n_traces

        try:
            self.fd, self.orig_pos = PyFile_Dup(file, "rb")
            if self.owner:
                # Advance fd to the start of the traces:
                pyi_fseek(self.fd, sizeof(int), SEEK_CUR)
        except Exception as err:
            self._close_file()
            raise err

    cdef SEGYTrace next_trace(self):
        if self.i == self.n_traces:
            raise StopIteration()
        cdef SEGYTrace out
        try:
            out = SEGYTrace.from_file_descriptor(self.fd)
        except Exception as err:
            # if something goes wrong reading in from the file descriptor
            # close myself and re-raise the error.
            self._close_file()
            raise err
        self.i += 1
        if self.i == self.n_traces:
            # The next request will raise a StopIteration so close myself now.
            self._close_file()
        return out

def _isfileobject(f):
    if not isinstance(f, (io.FileIO, io.BufferedReader, io.BufferedWriter)):
        return False
    try:
        f.fileno()
        return True
    except OSError:
        return False