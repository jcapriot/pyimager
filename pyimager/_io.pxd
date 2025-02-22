from libc.stdio cimport FILE

cdef extern from "_io.h":
    ctypedef int pyi_off_t

    FILE *pyi_fdopen(int fd, const char * mode)
    pyi_off_t pyi_lseek(int fd, pyi_off_t offset, int whence)
    int pyi_fseek(FILE *stream, pyi_off_t offset, int whence)
    pyi_off_t pyi_ftell(FILE *stream)


cdef FILE * PyFile_Dup(object file, char* mode, pyi_off_t *orig_pos)
cdef int PyFile_DupClose(object file, FILE* handle, pyi_off_t orig_pos)