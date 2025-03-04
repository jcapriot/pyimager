from libc.stdio cimport FILE
from libc.stdint cimport uint16_t, uint32_t, uint64_t, int16_t, int32_t, int64_t
from numpy cimport float32_t, float64_t


cdef extern from * nogil:
    """
    #ifndef _IO_HEADER_H
    #define _IO_HEADER_H

    #include <stdio.h>
    #include "Python.h"

    #if defined _MSC_VER && _MSC_VER >= 1900

        #include <stdlib.h>   // _set_thread_local_invalid_parameter_handler()
        /*
         * Macros to protect CRT calls against instant termination when passed an
         * invalid parameter (https://bugs.python.org/issue23524).
         */
        extern _invalid_parameter_handler _Py_silent_invalid_parameter_handler;
        #define _BEGIN_SUPPRESS_IPH { _invalid_parameter_handler _Py_old_handler = \
            _set_thread_local_invalid_parameter_handler(_Py_silent_invalid_parameter_handler);
        #define _END_SUPPRESS_IPH _set_thread_local_invalid_parameter_handler(_Py_old_handler); }

    #else

        #define _BEGIN_SUPPRESS_IPH
        #define _END_SUPPRESS_IPH

    #endif

    #ifdef _WIN32
    static FILE *spy_fdopen(int fd, const char *mode){
        FILE *f;
        _BEGIN_SUPPRESS_IPH
        f = _fdopen(fd, mode);
        _END_SUPPRESS_IPH
        return f;
    }
    #else
        #define spy_fdopen fdopen
    #endif

    /* 64 bit file position support, also on win-amd64. Issue gh-2256 */
    #if defined(_MSC_VER) && defined(_WIN64) && (_MSC_VER > 1400) || \
        defined(__MINGW32__) || defined(__MINGW64__)
        #include <io.h>
        #include <stdint.h>

        #define spy_off_t int64_t
        #define spy_fseek _fseeki64
        #define spy_ftell _ftelli64
        #define spy_lseek _lseeki64
    #else
        #ifdef HAVE_FSEEKO
            #define spy_fseek fseeko
        #else
            #define spy_fseek fseek
        #endif
        #ifdef HAVE_FTELLO
            #define spy_ftell ftello
        #else
            #define spy_ftell ftell
        #endif
        #include <sys/types.h>
        #ifndef _WIN32
            #include <unistd.h>
        #endif
        #define spy_lseek lseek
        #define spy_off_t off_t
    #endif

    #ifdef _MSC_VER
    #define spy_bswap_u16(x) _byteswap_ushort(x) //relying on ushort being 16 bits on mscv compiler
    #define spy_bswap_u32(x) _byteswap_ulong(x)  //relying on ulong being 32 bits on mscv compiler
    #define spy_bswap_u64(x) _byteswap_uint64(x)

    #elif defined(__APPLE__)

    // Mac OS X / Darwin features
    #include <libkern/OSByteOrder.h>
    #define spy_bswap2(x) OSSwapInt16(x)
    #define spy_bswap4(x) OSSwapInt32(x)
    #define spy_bswap8(x) OSSwapInt64(x)

    #else

    #ifdef DHAVE___BUILTIN_BSWAP16:
    #define spy_bswap_u16(x) __builtin_bswap16(x)
    #else
    #include "stdint.h"
    static inline uint16_t
    spy_bswap_u16(uint16_t x)
    {
        return ((x & 0xffu) << 8) | (x >> 8);
    }
    #endif

    #ifdef DHAVE___BUILTIN_BSWAP32:
    #define spy_bswap_u32(x) __builtin_bswap32(x)
    #else
    #include "stdint.h"
    static inline uint32_t
    spy_bswap_u32(uint32_t x)
    {
        return ((x & 0xffu) << 24) | ((x & 0xff00u) << 8) |
       ((x & 0xff0000u) >> 8) | (x >> 24);
    }
    #endif

    #ifdef DHAVE___BUILTIN_BSWAP64:
    #define spy_bswap_u64(x) __builtin_bswap64(x)
    #else
    #include "stdint.h"
    static inline uint64_t
    spy_bswap_u64(uint64_t x)
    {
        return ((x & 0xffULL) << 56) |
               ((x & 0xff00ULL) << 40) |
               ((x & 0xff0000ULL) << 24) |
               ((x & 0xff000000ULL) << 8) |
               ((x & 0xff00000000ULL) >> 8) |
               ((x & 0xff0000000000ULL) >> 24) |
               ((x & 0xff000000000000ULL) >> 40) |
               ( x >> 56);
    }
    #endif

    #endif

    #endif
    """
    ctypedef int spy_off_t

    FILE *spy_fdopen(int fd, const char * mode)
    spy_off_t spy_lseek(int fd, spy_off_t offset, int whence)
    int spy_fseek(FILE *stream, spy_off_t offset, int whence)
    spy_off_t spy_ftell(FILE *stream)

    uint16_t spy_bswap_u16(uint32_t x)
    uint32_t spy_bswap_u32(uint32_t x)
    uint64_t spy_bswap_u64(uint32_t x)


cdef (FILE *, spy_off_t) PyFile_Dup(object file, char* mode)
cdef int PyFile_DupClose(object file, FILE* handle, spy_off_t orig_pos)