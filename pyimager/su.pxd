from .cwp cimport cwp_String
from libc.stdio cimport FILE

cdef extern from "su.h" nogil:
    union Value: # storage for arbitrary type *
        char s[8];
        short h;
        unsigned short u;
        long l;
        unsigned long v;
        int i;
        unsigned int p;
        float f;
        double d;
        unsigned int U;
        unsigned int P;

    int READ_OK
    int WRITE_OK
    int EXEC_OK
    int FILE_OK

    int IS_DEPTH(char *str)
    int IS_COORD(char *str)

    # valpkge
    int vtoi(cwp_String type, Value val);
    long vtol(cwp_String type, Value val);
    float vtof(cwp_String type, Value val);
    double vtod(cwp_String type, Value val);
    int valcmp(cwp_String type, Value val1, Value val2);
    void printfval(cwp_String type, Value val);
    void fprintfval(FILE *stream, cwp_String type, Value val);
    void scanfval(cwp_String type, Value *valp);
    void atoval(cwp_String type, cwp_String keyval, Value *valp);
    void getparval(cwp_String name, cwp_String type, int n, Value *valp);
    Value valtoabs(cwp_String type, Value val);

    # segy coordinate scalar utilities
    short elco_scalar(int ncoords, double c[]);
    double from_segy_elco_multiplier(short segy_scalar);
    double to_segy_elco_multiplier(short segy_scalar); # reciprocal of from