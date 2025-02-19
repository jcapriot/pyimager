/*  #include "segy.h" */

#include "su.h"
#include "linterpd.h"

/* Linearly interpolate (double precision).                                           */
/* The following arguments are explained as if the inputs are offsets and times such  */
/* as a mute definition situation. But will also function for times,velocities and    */
/* other situations with an independent values array and a dependent values array.    */
/*                                                                                    */
/* Input arguments:                                                                   */
/*  offset = the offset of the desire output time                                     */
/*  offs   = array of offsets (must be in increasing order, but not checked herein)   */
/*  tims   = a time for each input offset                                             */
/*  nto    = number of offs and tims values (must be >0, can be =1)                   */
/*  mgtextr=0 no extrapolation at offset ends                                         */
/*         =1 extrapolate both lower and higher ends                                  */
/*         =2 extrapolate only at lower end                                           */
/*         =3 extrapolate only at higher                                              */
/*                                                                                    */
/* Output argument:                                                                   */
/*  time   = linearly interpolated time value for input offset value                  */

void linterpd(double offset,double *offs,double *tims,int nto,int mgtextr,double *time) {

  double wo = 0.;
  int n = 1;

  if(nto<2) {
    *time = tims[0];
    return;
  }

  if(offset <= offs[0]) {
    if(mgtextr==0 || mgtextr==3) {
      *time = tims[0];
      return;
    }
/*  n=1; but already initialed 1 (to avoid compiler warnings) */
  }
  else if(offset >= offs[nto-1]) {
    if(mgtextr==0 || mgtextr==2) {
      *time = tims[nto-1];
      return;
    }
    n = nto - 1;
  }
  else {
    n = bhighd(offs, nto, offset);
  }

  wo = (offs[n]-offset) / (offs[n]-offs[n-1]);
  *time = wo*tims[n-1] + (1.0-wo)*tims[n];

  return;

}

/* Binary search of double array.                                                     */
/*                                                                                    */
/* This is just a standard binary search. An important detail is what happens when    */ 
/* iguy matches an array value exactly. In that case the return value is 1 above the  */ 
/* exact match element. It also returns that same element number for exact+0.1 if     */ 
/* the next array value is greater than exact+0.1.                                    */ 
/*                                                                                    */
/* Input arguments:                                                                   */
/*  all  = array of double values in increasing order.                                */
/*  last = last element in array.                                                     */
/*  iguy = value to search for in array.                                              */
/*                                                                                    */
/* Return = element number                                                            */
/*                                                                                    */
int bhighd(double *all, int last, double iguy) {
  int mid; 
  int low = 0; 
  int high = last;
  while (low < high) {
    mid = low + (high - low) / 2; /* computed this way to prevent int overflow */ 
    if (iguy >= all[mid]) low = mid +1;
    else high = mid; 
  }
  return low; 
}
/* Binary search of int array. (not used by linterpd, but often needed nearby).       */
/*                                                                                    */
/* This is just a standard binary search. An important detail is what happens when    */ 
/* iguy matches an array value exactly. In that case the return value is 1 above the  */ 
/* exact match element. It also returns that same element number for exact+1 if       */ 
/* the next array value is greater than exact+1.                                      */ 
/*                                                                                    */
/* Input arguments:                                                                   */
/*  all  = array of int values in increasing order.                                   */
/*  last = last element in array.                                                     */
/*  iguy = value to search for in array.                                              */
/*                                                                                    */
/* Return = element number                                                            */
/*                                                                                    */
int bhighi(int *all, int last, int iguy) {
  int mid; 
  int low = 0;  
  int high = last;
  while (low < high) {
    mid = low + (high - low) / 2; /* computed this way to prevent int overflow */
    if (iguy >= all[mid]) low = mid +1; 
    else high = mid; 
  }
  return low; 
}

