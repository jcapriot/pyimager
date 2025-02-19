from .cwp cimport FileType
from posix.types cimport off_t
from libc.stdio cimport FILE, fpos_t

cdef extern from "par.h" nogil:
    # GLOBAL DECLARATIONS
    extern int xargc
    extern char ** xargv

    ctypedef int ssize_t

    # define structures for Hale's modeling
    struct ReflectorSegmentStruct:
        float x # x coordinate of segment midpoint
        float z # z coordinate of segment midpoint
        float s # x component of unit-normal-vector
        float c # z component of unit-normal-vector
    ctypedef ReflectorSegmentStruct ReflectorSegment
    struct ReflectorStruct:
        int ns # number of reflector segments
        float ds # segment length
        float a # amplitude of reflector
        ReflectorSegment * rs # array[ns] of reflector segments
    ctypedef ReflectorStruct Reflector

    struct WaveletStruct:
        int lw # length of wavelet
        int iw # index of first wavelet sample
        float * wv # wavelet sample values
    ctypedef WaveletStruct Wavelet

    # DEFINES

    # getpar macros
    void MUSTGETPARINT(char *name, int *p)
    void MUSTGETPARFLOAT(char *name, float *p)
    void MUSTGETPARSTRING(char *name, char **p)
    void MUSTGETPARDOUBLE(char *name, double *p)

    int STDIN # 0
    int STDOUT # 1
    int STDERR # 2

    # FUNCTION PROTOTYPES

    # getpar parameter parsing
    void initargs(int argc, char** argv)
    int getparint(char *name, int *p)
    int getparuint(char *name, unsigned int *p)
    int getparshort(char *name, short *p)
    int getparushort(char *name, unsigned short *p)
    int getparlong(char *name, long *p)
    int getparulong(char *name, unsigned long *p)
    int getparfloat(char *name, float *p)
    int getpardouble(char *name, double *p)
    int getparstring(char *name, char ** p)
    int getparstringarray(char *name, char ** p)
    int getnparint(int n, char *name, int *p)
    int getnparuint(int n, char *name, unsigned int *p)
    int getnparshort(int n, char *name, short *p)
    int getnparushort(int n, char *name, unsigned short *p)
    int getnparlong(int n, char *name, long *p)
    int getnparulong(int n, char *name, unsigned long *p)
    int getnparfloat(int n, char *name, float *p)
    int getnpardouble(int n, char *name, double *p)
    int getnparstring(int n, char *name, char ** p)
    int getnparstringarray(int n, char *name, char ** p)
    int getnpar(int n, char *name, char *type, void *ptr)
    int countparname(char *name)
    int countparval(char *name)
    int countnparval(int n, char *name)
    void checkpars()


    # For ProMAX
    void getPar(char *name, char *type, void *ptr)

    # errors and warnings
    void err(char *fmt, ...)
    void syserr(char *fmt, ...)
    void warn(char *fmt, ...)

    # self documentation
    void pagedoc(char *sdoc[])
    void requestdoc(int i, char *sdoc[])

    # system  subroutine calls with error trapping
    FILE *efopen(const char *file, const char *mode)
    FILE *efreopen(const char *file, const char *mode, FILE *stream1)
    FILE *efdopen(int fd, const char *mode)
    FILE *epopen(char *command, char *type)
    int efclose(FILE *stream)
    int epclose(FILE *stream)
    int efflush(FILE *stream)
    int eremove(const char *file)
    int erename(const char *oldfile, const char * newfile)
    int efseeko(FILE *stream, off_t offset, int origin)
    int efseek(FILE *stream, off_t offset, int origin)
    long eftell(FILE *stream)
    off_t eftello(FILE *stream)
    void erewind(FILE *stream)
    FILE *etmpstream(char *prefix)
    FILE *etmpfile()

    char *emkstemp(char *namebuffer)
    void *emalloc(size_t size)
    void *erealloc(void *memptr, size_t size)
    void *ecalloc(size_t count, size_t size)
    size_t efread(void *bufptr, size_t size, size_t count, FILE *stream)
    size_t efwrite(void *bufptr, size_t size, size_t count, FILE *stream)

    int efgetpos(FILE *stream, fpos_t *position)
    int efsetpos(FILE *stream, const fpos_t *position)

    # allocation with error trapping
    void * ealloc1(size_t n1, size_t size)
    void *erealloc1(void *v, size_t n1, size_t size)
    void ** ealloc2(size_t n1, size_t n2, size_t size)
    void ** *ealloc3(size_t n1, size_t n2, size_t n3, size_t size)
    void ** ** ealloc4(size_t n1, size_t n2, size_t n3, size_t n4, size_t size)
    void ** ** ealloc4(size_t n1, size_t n2, size_t n3, size_t n4, size_t size)
    void ** ** *ealloc5(size_t n1, size_t n2, size_t n3, size_t n4, size_t n5, size_t size)
    void ** ** ** ealloc6(size_t n1, size_t n2, size_t n3, size_t n4, size_t n5,
                          size_t n6, size_t size)

    int *ealloc1int(size_t n1)
    int *erealloc1int(int *v, size_t n1)
    int ** ealloc2int(size_t n1, size_t n2)
    int ** *ealloc3int(size_t n1, size_t n2, size_t n3)
    float *ealloc1float(size_t n1)
    float *erealloc1float(float *v, size_t n1)
    float ** ealloc2float(size_t n1, size_t n2)
    float ** *ealloc3float(size_t n1, size_t n2, size_t n3)

    int ** ** ealloc4int(size_t n1, size_t n2, size_t n3, size_t n4)
    int ** ** *ealloc5int(size_t n1, size_t n2, size_t n3, size_t n4, size_t n5)
    float ** ** ealloc4float(size_t n1, size_t n2, size_t n3, size_t n4)
    float ** ** *ealloc5float(size_t n1, size_t n2, size_t n3, size_t n4, size_t n5)
    float ** ** ** ealloc6float(size_t n1, size_t n2, size_t n3, size_t n4, size_t n5,
                                size_t n6)

    unsigned short ** ** *ealloc5ushort(size_t n1, size_t n2,
                                        size_t n3, size_t n4, size_t n5)
    unsigned char ** ** *ealloc5uchar(size_t n1, size_t n2,
                                      size_t n3, size_t n4, size_t n5)
    unsigned short ** ** ** ealloc6ushort(size_t n1, size_t n2,
                                          size_t n3, size_t n4, size_t n5, size_t n6)

    double *ealloc1double(size_t n1)
    double *erealloc1double(double *v, size_t n1)
    double ** ealloc2double(size_t n1, size_t n2)
    double ** *ealloc3double(size_t n1, size_t n2, size_t n3)
    complex *ealloc1complex(size_t n1)
    complex *erealloc1complex(complex *v, size_t n1)
    complex ** ealloc2complex(size_t n1, size_t n2)
    complex ** *ealloc3complex(size_t n1, size_t n2, size_t n3)

    # string to numeric conversion with error checking
    short eatoh(char *s)
    unsigned short eatou(char *s)
    int eatoi(char *s)
    unsigned int eatop(char *s)
    long eatol(char *s)
    unsigned long eatov(char *s)
    float eatof(char *s)
    double eatod(char *s)

    # file type checking
    FileType filestat(int fd)
    char *printstat(int fd)

    # Hale's modeling code
    void decodeReflectors(int *nrPtr,
                          float ** aPtr, int ** nxzPtr, float ** *xPtr, float ** *zPtr)
    int decodeReflector(char *string,
                        float *aPtr, int *nxzPtr, float ** xPtr, float ** zPtr)
    void breakReflectors(int *nr, float ** ar,
                         int ** nu, float ** *xu, float ** *zu)
    void makeref(float dsmax, int nr, float *ar,
                 int *nu, float ** xu, float ** zu, Reflector ** r)
    void raylv2(float v00, float dvdx, float dvdz,
                float x0, float z0, float x, float z,
                float *c, float *s, float *t, float *q)
    void addsinc(float time, float amp,
                 int nt, float dt, float ft, float *trace)
    void makericker(float fpeak, float dt, Wavelet ** w)

    # upwind eikonal stuff
    void eikpex(int na, float da, float r, float dr,
           float sc[], float uc[], float wc[], float tc[],
           float sn[], float un[], float wn[], float tn[])
    void ray_theoretic_sigma(int na, float da, float r, float dr,
                             float uc[], float wc[], float sc[],
                             float un[], float wn[], float sn[])
    void ray_theoretic_beta(int na, float da, float r, float dr,
                            float uc[], float wc[], float bc[],
                            float un[], float wn[], float bn[])
    void eiktam(float xs, float zs,
                int nz, float dz, float fz, int nx, float dx, float fx, float ** vel,
                float ** time, float ** angle, float ** sig, float ** beta)

    # smoothing routines
    void dlsq_smoothing(int nt, int nx, int ift, int ilt, int ifx, int ilx,
                   float r1, float r2, float rw, float ** traces)
    void SG_smoothing_filter(int np, int nl, int nr, int ld, int m, float *filter)
    void rwa_smoothing_filter(int flag, int nl, int nr, float *filter)
    void gaussian2d_smoothing(int nx, int nt, int nsx, int nst, float ** data)
    void gaussian1d_smoothing(int ns, int nsr, float *data)
    void smooth_histogram(int nintlh, float *pdf)
    void smooth_segmented_array(float *index, float *val, int n, int sm, int inc, int m)
    void smooth_1(float *x, float *z, float r, int n)

    # function minimization
    void bracket_minimum(float *ax, float *bx, float *cx, float *fa,
                    float *fb, float *fc, float (*func)(float))
    float golden_bracket(float ax, float bx, float cx,
                         float (*f)(float), float tol, float *xmin)
    float brent_bracket(float ax, float bx, float cx,
                        float (*f)(float), float tol, float *xmin)

    void linmin(float p[], float xi[], int n, float *fret, float (*func)())
    void powell_minimization(float p[], float ** xi, int n,
                             float ftol, int *iter, float *fret, float (*func)())

    # fractals
    float hausdorff_dimension(float *ar, int n, int minl, int maxl, int dl)

    # lincoeff -- linearized reflection coefficients
    # type definitions


    struct ErrorFlag:
        float iso[5]
        float upper[2]
        float lower[2]
        float glob "global"[4]
        float angle[4]

    # prototypes for functions defined

    float lincoef_Rp(float ang, float azim, float kappa, float *rpp, ErrorFlag *rp_1st, ErrorFlag *rp_2nd,
               int count)

    float lincoef_Rs(float ang, float azim, float kappa, float *rps1, float *rps2,
                     float *sv, float *sh, float *cphi, float *sphi, int i_hsp,
                     ErrorFlag *rsv_1st, ErrorFlag *rsv_2nd, ErrorFlag *rsh_1st, ErrorFlag *rsh_2nd, int count)

    float Iso_exact(int type, float vp1, float vs1, float rho1,
                    float vp2, float vs2, float rho2, float ang)

    int Phi_rot(float *rs1, float *rs2, int iso_plane, float pb_x, float pb_y, float pb_z, float gs1_x, float gs1_y,
                float gs1_z, float gs2_x, float gs2_y, float gs2_z, float *CPhi1, float *SPhi1, float *CPhi2, float
                *SPhi2)