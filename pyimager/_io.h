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
static FILE *pyi_fdopen(int fd, const char *mode){
    FILE *f;
    _BEGIN_SUPPRESS_IPH
    f = _fdopen(fd, mode);
    _END_SUPPRESS_IPH
    return f;
}
#else
    #define pyi_fdopen fdopen
#endif

/* 64 bit file position support, also on win-amd64. Issue gh-2256 */
#if defined(_MSC_VER) && defined(_WIN64) && (_MSC_VER > 1400) || \
    defined(__MINGW32__) || defined(__MINGW64__)
    #include <io.h>
    #include <stdint.h>

    #define pyi_off_t int64_t
    #define pyi_fseek _fseeki64
    #define pyi_ftell _ftelli64
    #define pyi_lseek _lseeki64
#else
    #ifdef HAVE_FSEEKO
        #define pyi_fseek fseeko
    #else
        #define pyi_fseek fseek
    #endif
    #ifdef HAVE_FTELLO
        #define pyi_ftell ftello
    #else
        #define pyi_ftell ftell
    #endif
    #include <sys/types.h>
    #ifndef _WIN32
        #include <unistd.h>
    #endif
    #define pyi_lseek lseek
    #define pyi_off_t off_t
#endif
#endif