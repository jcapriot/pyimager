# cython: embedsignature=True, language_level=3
# cython: linetrace=True
import io
import sys

from cpython.bytes cimport PyBytes_FromStringAndSize
from libc.stdio cimport FILE, fwrite, fread, SEEK_CUR
from libc.stdlib cimport malloc, free
from libc.string cimport memset, memcpy
cimport cython
cimport cpython.buffer as pybuf

from .io cimport PyFile_Dup, PyFile_DupClose, spy_off_t, spy_fseek

import os
from contextlib import nullcontext
import numpy as np

cdef spy_trace* new_trace(size_t n_sample, bint zero_fill=True) noexcept nogil:
    cdef spy_trace *tr = <spy_trace *> malloc(SPY_TRC_SIZE)
    memset(tr, 0, SPY_TRC_HDR_SIZE)
    if n_sample > 0:
        tr.hdr.n_sample = n_sample
        tr.data = <float *> malloc(sizeof(float) * n_sample)
        if zero_fill:
            memset(tr.data, 0, sizeof(float) * n_sample)
    else:
        tr.data = NULL
    return tr

cdef spy_trace* copy_of(spy_trace *tr_in, bint copy_data=True) noexcept nogil:
    cdef spy_trace *tr_copy = <spy_trace *> malloc(SPY_TRC_SIZE)
    memcpy(tr_copy, tr_in, SPY_TRC_HDR_SIZE)
    cdef size_t n_sample = tr_in.hdr.n_sample
    if n_sample > 0:
        tr_copy.data = <float *> malloc(sizeof(float) * n_sample)
        memcpy(tr_copy.data,  tr_in.data, sizeof(float) * n_sample)
    else:
        tr_copy.data = NULL
    return tr_copy

cdef void del_trace(spy_trace *tp, bint del_data) noexcept nogil:
    if (tp != NULL) and del_data:
        if tp.data != NULL:
            free(tp.data)
        tp.data = NULL
    free(tp)

_SAMPLING_MAP = {'s': SPY_SMPLNG_UNIT_SEC, 'm':SPY_SMPLNG_UNIT_METER}
_DOMAIN_MAP = {'unit':SPY_SMPLNG_DOM_UNIT, 'fourier':SPY_SMPLNG_DOM_FOURIER}

@cython.final
cdef class Trace:

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
        double d_sample,
        double sample_start=0.0,
        tx_loc=None,
        rx_loc=None,
        sampling_unit='s',
        sampling_domain='unit',

    ):
        self.trace_data = np.require(data, dtype=np.float32, requirements='C')
        cdef spy_trace *tr = new_trace(0)
        cdef spy_trace_header *hdr = &tr.hdr
        if tr is NULL:
            raise MemoryError("Unable to allocate trace.")

        tr.data = &self.trace_data[0]
        hdr.n_sample = self.trace_data.shape[0]
        hdr.d_sample = d_sample
        hdr.sample_start = sample_start

        if tx_loc is not None:
            hdr.tx_loc = tx_loc

        if rx_loc is not None:
            hdr.rx_loc = rx_loc

        try:
            hdr.sampling_unit = _SAMPLING_MAP[sampling_unit]
        except KeyError:
            raise KeyError("Trace expected `sampling_unit` to be one of 's' (seconds) or 'm' (meters).")

        try:
            hdr.sampling_domain = _DOMAIN_MAP[sampling_domain]
        except KeyError:
            raise KeyError("Trace expected `sampling_domain` to be one of 'unit' (seconds) or 'fourier' (meters).")

        self.tr = tr
        self.trace_owner = True
        self.trace_data_owner = False

    def __len__(self):
        return self.tr.hdr.n_sample

    @property
    def n_sample(self):
        return self.tr.hdr.n_sample

    @property
    def d_sample(self):
        return self.tr.hdr.d_sample

    @staticmethod
    cdef Trace from_trace(spy_trace *tr, bint trace_owner=False, bint data_owner=False):
        cdef Trace cy_trace = Trace.__new__(Trace)
        cy_trace.tr = tr
        cy_trace.trace_data = <float[:tr.hdr.n_sample]> tr.data
        cy_trace.trace_owner = trace_owner
        cy_trace.data_owner = data_owner
        return cy_trace

    @staticmethod
    cdef Trace from_file_descriptor(FILE *fd):

        cdef spy_trace *tr = new_trace(0)
        if tr is NULL:
            raise MemoryError("Unable to allocate trace structure.")
        cdef:
            size_t n_read
            size_t n_sample

        n_read = fread(tr, SPY_TRC_HDR_SIZE, 1, fd)
        if n_read != 1:
            del_trace(tr, 1)
            raise IOError("Unable to read trace header from file.")

        n_sample = tr.hdr.n_sample
        tr.data = <float *> malloc(sizeof(float)*n_sample)
        if tr.data is NULL:
            raise MemoryError("Unable to allocate trace data.")
        n_read = fread(tr.data, sizeof(float), n_sample, fd)
        if n_read != n_sample:
            free(tr.data)
            raise IOError("Unable to read expected number of trace samples.")

        return Trace.from_trace(tr, True, True)

    cdef to_file_descriptor(self, FILE *fd):
        cdef int n_write = fwrite(self.tr, SPY_TRC_HDR_SIZE, 1, fd)
        if n_write != 1:
            raise IOError("Error writing trace header to file.")

        n_write = fwrite(self.tr.data, sizeof(float), self.n_sample, fd)
        if n_write != self.n_sample:
            raise IOError("Error writing trace data to file.")

    def __getbuffer__(self, Py_buffer *buffer, int flags):
        cdef Py_ssize_t itemsize = sizeof(float)

        buffer.obj = self
        buffer.buf = <void *> self.tr.data
        buffer.len = self.n_sample
        buffer.itemsize = itemsize
        buffer.ndim = 1

        buffer.shape = self.trace_data.shape
        buffer.strides = self.trace_data.strides
        buffer.readonly = 0

        if flags & pybuf.PyBUF_FORMAT:
            buffer.format = 'f'
        else:
            buffer.format = NULL

        buffer.internal = NULL
        buffer.suboffsets = NULL

    def __releasebuffer__(self, Py_buffer *buffer):
        pass

cdef class TraceCollection:
    def __cinit__(self):
        self.file = None
        self.traces = None
        self.iterator = None
        self.ntr = 0


    def __init__(self, trace_data, d_sample, **kwargs):
        n_tr = len(trace_data)
        traces = []
        for trace in trace_data:
            traces.append(Trace(trace, d_sample=d_sample, **kwargs))
        self.traces = traces
        self.ntr = len(traces)

    def __len__(self):
        return self.ntr

    @property
    def n_trace(self):
        return self.ntr

    @classmethod
    def from_file(cls, filename):
        cdef:
            Trace trace
            FILE *fd
            bint file_owner
            spy_off_t orig_pos = 0
            int ntr = 0

        if hasattr(filename, 'read'):
            ctx = nullcontext(filename)
        else:
            ctx = open(os.fspath(filename), "rb")

        with ctx as f:
            ntr = int.from_bytes(f.read(sizeof(ntr)), byteorder=sys.byteorder)

        cdef TraceCollection new_segy = TraceCollection.__new__(TraceCollection)

        new_segy.file = filename
        new_segy.ntr = ntr

        return new_segy

    @staticmethod
    cdef TraceCollection from_trace_iterator(BaseTraceIterator iterator):
        cdef TraceCollection collect = TraceCollection.__new__(TraceCollection)
        collect.iterator = iterator
        collect.ntr = iterator.n_traces
        return collect

    @property
    def on_disk(self):
        return self.file is not None

    @property
    def is_iterator(self):
        return self.iterator is not None

    @property
    def in_memory(self):
        return self.traces is not None

    def __iter__(self):
        if self.on_disk:
            return _FileTraceIterator(self.file, self.ntr)
        elif self.in_memory:
            return _MemoryTraceIterator(self.traces)
        elif self.is_iterator:
            return self.iterator
        else:
            raise TypeError('TraceCollection file is not on disk, in memory, nor from an iterator.')

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
            Trace trace
            FILE *fd
            bint file_owner
            spy_off_t orig_pos = 0


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
            Trace trace

        with ctx as stream_ctx:
            stream_ctx.write(PyBytes_FromStringAndSize(<char *> &self.ntr, sizeof(self.ntr)))
            for trace in self:
                stream_ctx.write(PyBytes_FromStringAndSize(<char *> trace.tr, SPY_TRC_HDR_SIZE))
                stream_ctx.write(PyBytes_FromStringAndSize(<char *> trace.tr.data, sizeof(float)*trace.n_sample))
            stream_ctx.flush()
        if self.is_iterator:
            # the above will consume the iterator if it came from one.
            self.iterator = None
            # otherwise, don't do anything to the underlying object



cdef class BaseTraceIterator:
    def __cinit__(self):
        self.i = 0
        self.n_traces = 0

    cdef Trace next_trace(self):
        raise NotImplementedError(f"cdef next_trace is not implemented on {type(self)}.")

    def __next__(self):
        return self.next_trace()

    def __iter__(self):
        return self

    def to_memory(self):
        return TraceCollection.from_trace_iterator(self).to_memory()

    def to_file(self, filename):
        return TraceCollection.from_trace_iterator(self).to_file(filename)

    def to_stream(self, stream):
        TraceCollection.from_trace_iterator(self).to_stream(stream)


cdef class _MemoryTraceIterator(BaseTraceIterator):
    cdef:
        list traces

    def __init__(self, list traces):
        self.traces = traces
        self.n_traces = len(traces)

    cdef Trace next_trace(self):
        if self.i == self.n_traces:
            raise StopIteration()
        cdef Trace out = self.traces[self.i]
        self.i += 1
        return out


cdef class _FileTraceIterator(BaseTraceIterator):
    cdef:
        FILE *fd
        bint owner
        object file
        spy_off_t orig_pos

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
                spy_fseek(self.fd, sizeof(int), SEEK_CUR)
        except Exception as err:
            self._close_file()
            raise err

    cdef Trace next_trace(self):
        if self.i == self.n_traces:
            raise StopIteration()
        cdef Trace out
        try:
            out = Trace.from_file_descriptor(self.fd)
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