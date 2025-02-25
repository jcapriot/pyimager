from cpython.object cimport PyObject_AsFileDescriptor
from libc.stdio cimport FILE, fclose, SEEK_SET
from libc.limits cimport INT_MIN, INT_MAX
import os
import io

"""
This module is meant to handle getting a C FILE pointer to an open python file object. It's contents
were adapted from numpy's npy_3kcompat.h file. As suggested in that file, they recommend copying that
file due to there lack of backwards compatibility guarantees.

Permanent link to the version of the reference file is at:

https://github.com/numpy/numpy/blob/a1f2d582f84878b6c67bd641fa671a4cf868fe8c/numpy/_core/include/numpy/npy_3kcompat.h
"""

# The docstring below is raw C code necessary for Windows
cdef extern from "_io.h":
    """
    #ifdef _MSC_VER

    #include <stdlib.h>

    #if _MSC_VER >= 1900
    /* _io.h uses this function in the _Py_BEGIN/END_SUPPRESS_IPH
     * macros. It does not need to be defined when building using MSVC
     * earlier than 14.0 (_MSC_VER == 1900).
     */

    static void __cdecl _silent_invalid_parameter_handler(
        wchar_t const* expression,
        wchar_t const* function,
        wchar_t const* file,
        unsigned int line,
        uintptr_t pReserved) { }

    _invalid_parameter_handler _Py_silent_invalid_parameter_handler = _silent_invalid_parameter_handler;

    #endif

    #endif
    """
    ctypedef int pyi_off_t

    FILE *pyi_fdopen(int fd, const char * mode)
    pyi_off_t pyi_lseek(int fd, pyi_off_t offset, int whence)
    int pyi_fseek(FILE *stream, pyi_off_t offset, int whence)
    pyi_off_t pyi_ftell(FILE *stream)


cdef FILE * PyFile_Dup(object file, char* mode, pyi_off_t *orig_pos):
    cdef:
        int fd, fd2
        Py_ssize_t fd2_tmp
        pyi_off_t pos
        FILE *handle

    file.flush()

    fd = PyObject_AsFileDescriptor(file)
    if fd == -1:
        return NULL
    fd2_tmp = os.dup(fd)
    if fd2_tmp < INT_MIN or fd2_tmp > INT_MAX:
        raise IOError("Getting an 'int' from os.dup() failed")

    fd2 = <int> fd2_tmp
    handle = pyi_fdopen(fd2, mode)
    orig_pos[0] = pyi_ftell(handle)
    if orig_pos[0] == -1:
        if isinstance(file, io.RawIOBase):
            return handle
        else:
            raise IOError("obtaining file position failed")

    # raw handle to the Python-side position
    try:
        pos = file.tell()
    except Exception:
        fclose(handle)
    if pyi_fseek(handle, pos, SEEK_SET) == -1:
        fclose(handle)
        raise IOError("seeking file failed")
    return handle

cdef int PyFile_DupClose(object file, FILE* handle, pyi_off_t orig_pos):
    cdef:
        int fd
        pyi_off_t position = pyi_ftell(handle)
    fclose(handle)

    # Restore original file handle position,
    fd = PyObject_AsFileDescriptor(file)
    if fd == -1:
        return -1
    if pyi_lseek(fd, orig_pos, SEEK_SET) == -1:
        if isinstance(file, io.RawIOBase):
            return 0
        else:
            raise IOError("seeking file failed")
    if position == -1:
        raise IOError("obtaining file position failed")

    # Seek Python-side handle to the FILE* handle position
    file.seek(position)
    return 0
