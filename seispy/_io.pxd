from libc.stdio cimport FILE

cdef extern from "_io.h":
    ctypedef int spy_off_t

    FILE *spy_fdopen(int fd, const char * mode)
    spy_off_t spy_lseek(int fd, spy_off_t offset, int whence)
    int spy_fseek(FILE *stream, spy_off_t offset, int whence)
    spy_off_t spy_ftell(FILE *stream)


cdef (FILE *, spy_off_t) PyFile_Dup(object file, char* mode)
cdef int PyFile_DupClose(object file, FILE* handle, spy_off_t orig_pos)