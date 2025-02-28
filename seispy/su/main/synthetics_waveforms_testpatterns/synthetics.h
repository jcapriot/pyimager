#ifndef SU_SYNTHETICS_H
#define SU_SYNTHETICS_H
#include "segy.h"  // segy
#include "par.h" // Reflector and Wavelet

/* FUNCTION PROTOTYPES */
#ifdef __cplusplus /* if C++, specify external linkage to C functions */
extern "C" {
#endif

void susynlv_filltrace(
    segy *tr, int shots, int kilounits, int tracl,
    float fxs, int ixsm, float dxs,
    int ixo, float *xo, float dxo, float fxo,
    float dxm, float fxm, float dxsm,
    float v00, float dvdx, float dvdz, int ls, int er, int ob, Wavelet *w,
    int nr, Reflector *r, int nt, float dt, float ft
);
#ifdef __cplusplus /* if C++, end external linkage specification */
}
#endif
#endif