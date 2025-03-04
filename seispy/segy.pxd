cdef extern from "segy.h" nogil:
    int SU_NFLTS

    ctypedef struct segy:
        int tracl
        int tracr
        int fldr
        int tracf
        int ep
        int cdp
        int cdpt
        short trid
        short nvs
        short nhs
        short duse
        int offset
        int gelev
        int selev
        int sdepth
        int gdel
        int sdel
        int swdep
        int gwdep
        short scalel
        short scalco
        int  sx
        int  sy
        int  gx
        int  gy
        short counit
        short wevel
        short swevel
        short sut
        short gut
        short sstat
        short gstat
        short tstat
        short laga
        short lagb
        short delrt
        short muts
        short mute
        unsigned short ns
        unsigned short dt
        short gain
        short igc
        short igi
        short corr
        short sfs
        short sfe
        short slen
        short styp
        short stas
        short stae
        short tatyp
        short afilf
        short afils
        short nofilf
        short nofils
        short lcf
        short hcf
        short lcs
        short hcs
        short year
        short day
        short hour
        short minute
        short sec
        short timbas
        short trwf
        short grnors
        short grnofr
        short grnlof
        short gaps
        short otrav
        float d1
        float f1
        float d2
        float f2
        float ungpow
        float unscale
        int ntr
        short mark
        short shortpad
        short *unass
        float *data

    ctypedef struct hdr:
        pass

    struct bhed:
        int jobid
        int lino
        int reno
        short ntrpr
        short nart
        unsigned short hdt
        unsigned short dto
        unsigned short hns
        unsigned short nso
        short format
        short fold
        short vscode
        short hsfs
        short hsfe
        short hslen
        short hstyp
        short schn
        short hstas
        short hstae
        short htatyp
        short hcorr
        short bgrcv
        short rcvm
        short mfeet
        short polyt
        short vpol
        short hunass[170]

    int TOTHER
    int TUNK
    int TREAL
    int TDEAD
    int TDUMMY
    int TBREAK
    int UPHOLE
    int SWEEP
    int TIMING
    int WBREAK
    int NFGUNSIG
    int FFGUNSIG
    int SPSENSOR
    int TVERT
    int TXLIN
    int TINLIN
    int ROTVERT
    int TTRANS
    int TRADIAL
    int VRMASS
    int VBASS
    int VEGF
    int VREF

    int ACOR
    int FCMPLX
    int FUNPACKNYQ

    int FTPACK
    int TCMPLX
    int FAMPH
    int TAMPH
    int REALPART
    int IMAGPART
    int AMPLITUDE
    int PHASE
    int KT
    int KOMEGA
    int ENVELOPE
    int INSTRFEQ
    int LOGAMPLITUDE
    int CEPSTRUM
    int TRID_DEPTH
    int CHARPACK
    int SHORTPACK

    int ISSEISMIC(int id)

    int MAXTraceCollection
    int SU_NKEYS
    int HDRBYTES

    # get trace and put trace
    int fgettr(FILE *fp, segy *tp)
    int fvgettr(FILE *fp, segy *tp)
    void fputtr(FILE *fp, segy *tp)
    void fvputtr(FILE *fp, segy *tp)
    int fgettra(FILE *fp, segy *tp, int itr)

    # get gather and put gather
    segy **fget_gather(FILE *fp, cwp_String *key,cwp_String *type,Value *n_val,
                            int *nt,int *ntr, float *dt,int *first)
    segy **get_gather(cwp_String *key, cwp_String *type, Value *n_val,
                int *nt, int *ntr, float *dt, int *first)
    segy **fput_gather(FILE *fp, segy **rec,int *nt, int *ntr)
    segy **put_gather(segy **rec,int *nt, int *ntr)

    # hdrpkge
    void gethval(const segy *tp, int index, Value *valp)
    void puthval(segy *tp, int index, Value *valp)
    void getbhval(const bhed *bhp, int index, Value *valp)
    void putbhval(bhed *bhp, int index, Value *valp)
    void gethdval(const segy *tp, char *key, Value *valp)
    void puthdval(segy *tp, char *key, Value *valp)
    char *hdtype(const char *key)
    char *getkey(const int index)
    int getindex(const char *key)
    void swaphval(segy *tp, int index)
    void swapbhval(bhed *bhp, int index)
    void printheader(const segy *tp)

    void tabplot(segy *tp, int itmin, int itmax)