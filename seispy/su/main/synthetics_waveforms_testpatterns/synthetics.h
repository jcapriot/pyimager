#ifndef SU_SYNTHETICS_H
#define SU_SYNTHETICS_H
#include "spy_trace.h"  // spy_trace
#include "par.h" // Reflector and Wavelet

/* FUNCTION PROTOTYPES */
#ifdef __cplusplus /* if C++, specify external linkage to C functions */
extern "C" {
#endif

void susynlv_filltrace(spy_trace *trace, float v00, float dvdx, float dvdz,
	int ls, int er, int ob, Wavelet *w, int nr, Reflector *r, int lhd, int nhd, float *hd);
#ifdef __cplusplus /* if C++, end external linkage specification */
}
#endif
#endif