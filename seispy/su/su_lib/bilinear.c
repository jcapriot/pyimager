/*  #include "segy.h" */

#include "su.h"
#include "bilinear.h"
#include "linterpd.h"

/* Find the 4 storage locations associated with the cdp's grid index locations (kigi,kigc).  */
/* The 4 locations are: mgixo,mgcxo and mgixo-1,mgcxo and mgixo,mgcxo-1 and mgixo-1,mgcxo-1. */
/* These 4 locations do not always surround the location to be found. For instance, below    */
/* the minimum igi value of the input functions, mgixo is returned 1 anyway (not 0).         */
/* But the wi weight is returned as 1.0 so only the lowest igi function contributes to the   */
/* resulting value from binterpapply (except if extrapolation enabled).                      */
/* Similarly, for locations above the maximum igi value of input functions, mgixo is         */
/* returned as the maximum BUT wi weight is returned as 0 so only the highest function       */
/* contributes to the resulting value from binterpapply (except if extrapolation enabled).   */
/*                                                                                           */
/* Input arguments:                                                                          */
/*                                                                                           */
/* kigi      igi number of cdp (the 3D grid inline location of the cdp)                      */
/*                                                                                           */
/* mgi       array of igi numbers of the functions (3D grid inline locations)                */
/*                                                                                           */
/* mgi_tot   number of values in mgi array                                                   */
/*                                                                                           */
/* mgiextr=0 no extrapolation at igi ends                                                    */
/*        =1 extrapolate both lower and higher ends                                          */
/*        =2 extrapolate only at lower end                                                   */
/*        =3 extrapolate only at higher                                                      */
/*                                                                                           */
/* kigc      igc number of cdp (the 3D grid crossline location of the cdp)                   */
/*                                                                                           */
/* mgc       array of igc numbers of the functions (3D grid crossline locations)             */
/*                                                                                           */
/* mgc_tot   number of values in mgc array                                                   */
/*                                                                                           */
/* mgcextr=0 no extrapolation at igc ends                                                    */
/*        =1 extrapolate both lower and higher ends                                          */
/*        =2 extrapolate only at lower end                                                   */
/*        =3 extrapolate only at higher                                                      */
/*                                                                                           */
/*                                                                                           */
/* Output arguments:                                                                         */
/*                                                                                           */
/* mgixo     mgi element number where mgi[mgixo] is usually greater than kigi (there is      */
/*           some trickyness here, read the note below)                                      */
/*                                                                                           */
/* mgcxo     mgc element number where mgc[mgcxo] is usually greater than kigc (there is      */
/*           some trickyness here, read the note below)                                      */
/*                                                                                           */
/* wi        weight in the igi direction.                                                    */
/*           This weight should be applied to the TWO functions associated with mgixo-1      */
/*           and (1.-wi) should be applied to the TWO functions associated with mgixo.       */
/*                                                                                           */
/* wc        weight in the igc direction                                                     */
/*           This weight should be applied to the TWO functions associated with mgcxo-1      */
/*           and (1.-wc) should be applied to the TWO functions associated with mgcxo.       */
/*                                                                                           */

void binterpfind(int kigi, int *mgi, int mgi_tot, int mgiextr,
                 int kigc, int *mgc, int mgc_tot, int mgcextr,
                 int *mgixo, int *mgcxo, double *wi, double *wc) {

  int mgix = 0;
  int mgcx = 0;

  if(mgi_tot==1 && mgc_tot==1) { 
    *wi    = 1.;
    *wc    = 0.;
    *mgixo = 1; 
    *mgcxo = 1;
    return;
  }

/* Note the trickyness here. We never return an mgix=0 because other  */
/* code is going to use mgix-1 for the lower "surrounding" location.  */
/* So, for kigi less than lowest, we set mgix=1. But we reset kigi so */
/* subsequent computation puts all weight on the values at mgi[0]     */
/* (unless we want to extrapolate the low end).                       */
/*                                                                    */
/* But when kigi is greater than highest, we set mgix to highest and  */
/* reset kigi so that subsequent computation puts all weight on the   */
/* values at the highest (unless we want to extrapolate the high end).*/

  if(kigi<=mgi[0]) {
    mgix = 1;
    if(mgiextr==0 || mgiextr==3) kigi = mgi[0];
  }
  else if(kigi>=mgi[mgi_tot-1]) { 
    mgix = mgi_tot - 1;     
    if(mgiextr==0 || mgiextr==2) kigi = mgi[mgi_tot-1];
  }
  else {
    mgix = bhighi(mgi, mgi_tot, kigi);
  }

  if(mgc_tot==1) {
    *wi    = ((double)(mgi[mgix]-kigi)) / ((double)(mgi[mgix]-mgi[mgix-1]));
    *wc    = 0.; 
    *mgixo = mgix;
    *mgcxo = 1;
    return;
  }

/* Same trickyness next as explained above.  */

  if(kigc<=mgc[0]) {
    mgcx = 1;
    if(mgcextr==0 || mgcextr==3) kigc = mgc[0];
  }
  else if(kigc>=mgc[mgc_tot-1]) {
    mgcx = mgc_tot - 1;    
    if(mgcextr==0 || mgcextr==2) kigc = mgc[mgc_tot-1];
  }
  else {
    mgcx = bhighi(mgc, mgc_tot, kigc);
  }

  if(mgi_tot==1) { 
    *wi    = 0.;
    *wc    = ((double)(mgc[mgcx]-kigc)) / ((double)(mgc[mgcx]-mgc[mgcx-1]));
    *mgixo = 1;
    *mgcxo = mgcx;
    return;
  }

  *wi    = ((double)(mgi[mgix]-kigi)) / ((double)(mgi[mgix]-mgi[mgix-1]));
  *wc    = ((double)(mgc[mgcx]-kigc)) / ((double)(mgc[mgcx]-mgc[mgcx-1]));
  *mgixo = mgix;
  *mgcxo = mgcx;

  return;
}

/* Apply weights and sum values associated with kigi,kigc from binterpfind   */
/*                                                                           */
/* In binterpfind above, mgix and mgcx are the inline and crossline locations*/
/* computed for each direction seperately. binterpfind always returns the    */
/* mgix and mgcx values that are the highest of the 2 (near) locations.      */
/* So, if mgi has 10 locations, mgix is only returned from 1 to 9, not 0.    */
/* The mgix and mgcx values must then be used to compute the 4 locations     */
/* of the functions that contribute to the weighted output sum done herein.  */
/*                                                                           */
/* The following example from subinqcsv shows its computation:               */
/* (but this, of course, depends on how you have stored your values)         */
/*                                                                           */
/*   mgi_totdeg = mgi_tot;                                                   */
/*   if(mgi_tot==1 || mgc_tot==1) mgi_totdeg = 0;                            */
/*   ndxi = 0;                                                               */
/*   ndxc = 0;                                                               */
/*   mdxi = 0;                                                               */
/*   mdxc = 0;                                                               */
/*                                                                           */
/*   if(ncdp>1) {                                                            */
/*      binterpfind(kigi,mgi,mgi_tot,mgiextr,kigc,mgc,mgc_tot,mgcextr,       */
/*                  &mgix,&mgcx,&wi,&wc);                                    */
/*                                                                           */
/*      ndxi = mgix + mgi_tot * (mgcx-1);                                    */
/*      ndxc = ndxi + mgi_totdeg;                                            */
/*      mdxi = ndxi-1;                                                       */
/*      mdxc = ndxc-1;                                                       */
/*    }                                                                      */
/*                                                                           */
/*    binterpapply(RecInfo[mdxi].dlots, RecInfo[ndxi].dlots, mgi_tot, wi,    */
/*                 RecInfo[mdxc].dlots, RecInfo[ndxc].dlots, mgc_tot, wc,    */
/*                 klast,dswap);                                             */
/*                                                                           */
/* Note that for the degenerate cases of mgi_tot=1 or mgc_tot=1 the          */
/* mgi_totdeg=0, which results in ndxc=ndxi, which in turn means the second  */
/* two functions passed to binterpapply are the same as first two (which     */
/* works because either weight wi or wc will be 0.0). Similarly, if ncdp<2   */
/* ndxi,ndxc,mdxi,mdxc remain 0 (binterpapply is passed same array 4 times). */
/*                                                                           */
/*                                                                           */
/* Input arguments:                                                          */
/*                                                                           */
/* lwitims - array associated with a location returned by binterpfind        */
/*                                                                           */
/* hiitims - array associated with a location returned by binterpfind        */
/*                                                                           */
/* mgi_tot - number of mgi values (just a flag herein, 1 is handled special).*/
/*                                                                           */
/* wi      - weight associated with igi direction (returned by binterpfind)  */
/*                                                                           */
/* lwctims - array associated with a location returned by binterpfind        */
/*                                                                           */
/* hictims - array associated with a location returned by binterpfind        */
/*                                                                           */
/* mgc_tot - number of mgc values (just a flag herein, 1 is handled special).*/
/*                                                                           */
/* wc      - weight associated with igc direction (returned by binterpfind)  */
/*                                                                           */
/* lwinto  - length of the 4 input arrays (lwitims,hiitims,lwctims,hictims)  */
/*           and the output array (valsout).                                 */
/*                                                                           */
/*                                                                           */
/* Output Argument:                                                          */
/*                                                                           */
/* valsout - output array containing weighted sum of the 4 input arrays      */
/*           (length must be lwinto or greater)                              */
/*                                                                           */
/* ------------------------------------------------------------------------- */
/* NOTE that program sunmocsv uses binterpapply whereas program sumutecsv    */
/*      uses binterpvalue. The reason is that sunmocsv pre-computes and      */
/*      stores arrays of velocities (a velocity for each sample time).       */
/*      But sumutecsv computes the mute time on-the-fly for the offset       */
/*      from each trace.                                                     */
/*                                                                           */
void binterpapply(double *lwitims, double *hiitims, int mgi_tot, double wi, 
                  double *lwctims, double *hictims, int mgc_tot, double wc,
                  int lwinto, double *valsout) {

  double aw = 0.;
  double bw = 0.;
  double cw = 0.;
  double dw = 0.;
  int n = 0;

  if(mgi_tot==1 && mgc_tot==1) { 
    for(n=0; n<lwinto; n++) valsout[n] = lwitims[n];
    return;
  }

  for(n=0; n<lwinto; n++) valsout[n] = 0.;

  if(mgc_tot==1) {
    if(wi != 0.) { /* because of extrapolation options, check exactly 0 */
      for(n=0; n<lwinto; n++) valsout[n] += wi*lwitims[n];
    }
    if(wi != 1.) { /* because of extrapolation options, check exactly 1 */
      for(n=0; n<lwinto; n++) valsout[n] += (1.0-wi)*hiitims[n];
    }
    return;
  }

  if(mgi_tot==1) { 
    if(wc != 0.) {
      for(n=0; n<lwinto; n++) valsout[n] += wc*lwctims[n];
    }
    if(wc != 1.) {
      for(n=0; n<lwinto; n++) valsout[n] += (1.0-wc)*hictims[n];
    }
    return;
  }

/* The 4 point weighting equation looks like this:           */  
/*  *valsout =  wc      * (wi*timea + (1.0-wi)*timeb)        */  
/*           + (1.0-wc) * (wi*timec + (1.0-wi)*timed);       */
/*                                                           */
/* But reduce some brackets and it looks like this:          */  
/*  *valsout =  wc*wi*timea + wc*(1.0-wi)*timeb              */  
/*           + (1.0-wc)*wi*timec + (1.0-wc)*(1.0-wi)*timed;  */
/*                                                           */
/* So we can isolate the weight factors needed for each of   */  
/* the 4 locations, as follows:                              */  

  aw = wc*wi;
  bw = wc*(1.0-wi);
  cw = (1.0-wc)*wi;
  dw = (1.0-wc)*(1.0-wi);

/* Which means we do not have to sum it when we know the     */  
/* corresponding weight is zero. This may seem like it will  */  
/* only save a small amount of CPU time but remember that    */  
/* most situations have many locations outside of the area   */  
/* that is completely surrounded by input function locations.*/  
/* When outside surrounded area, and extrapolation is not    */  
/* enabled, binterpfind has produced wi=0,1 and/or wc=0,1    */  
/* Even when extrapolation is enabled, there are many        */  
/* locations which are exactly on an aligned rectangle edge  */  
/* and will therefore still have 0 or 1 weight.              */  

  if(aw != 0.) {
    for(n=0; n<lwinto; n++) valsout[n] += aw*lwitims[n];
  }
  if(bw != 0.) {
    for(n=0; n<lwinto; n++) valsout[n] += bw*hiitims[n];
  }
  if(cw != 0.) {
    for(n=0; n<lwinto; n++) valsout[n] += cw*lwctims[n];
  }
  if(dw != 0.) {
    for(n=0; n<lwinto; n++) valsout[n] += dw*hictims[n];
  }

  return;
}

/* Compute mute time for an offset distance.                                 */
/* (Note that offset distance and mute time are just the typical way this    */
/*  routine is used so I am explaining it that way.)                         */
/*                                                                           */
/* Use the storage locations and weights found by binterpfind and compute    */
/* mute time related to the input offset (by linear interpolation within     */
/* those functions and then by applying the input wi,wc weigths).            */
/*                                                                           */
/*                                                                           */
/* Input arguments:                                                          */
/*                                                                           */
/* lwioffs - offset array at low igi location found by binterpfind           */
/*                                                                           */
/* lwitims -  time  array at low igi location found by binterpfind           */
/*                                                                           */
/* lwinto  - number of values in lwioffs,lwitims arrays                      */
/*                                                                           */
/* hwioffs - offset array at high igi location found by binterpfind          */
/*                                                                           */
/* hwitims -  time  array at high igi location found by binterpfind          */
/*                                                                           */
/* hwinto  - number of values in hwioffs,hwitims arrays                      */
/*                                                                           */
/* mgi_tot - number of mgi values (just a flag herein, 1 is handled special).*/
/*                                                                           */
/* wi      - weight associated with igi direction (returned by binterpfind)  */
/*                                                                           */
/* lwcoffs - offset array at low igc location found by binterpfind           */
/*                                                                           */
/* lwctims -  time  array at low igc location found by binterpfind           */
/*                                                                           */
/* lwcnto  - number of values in lwcoffs,lwctims arrays                      */
/*                                                                           */
/* hwcoffs - offset array at high igc location found by binterpfind          */
/*                                                                           */
/* hwctims -  time  array at high igc location found by binterpfind          */
/*                                                                           */
/* hwcnto  - number of values in hwcoffs,hwctims arrays                      */
/*                                                                           */
/* mgc_tot - number of mgc values (just a flag herein, 1 is handled special).*/
/*                                                                           */
/* wc      - weight associated with igc direction (returned by binterpfind)  */
/*                                                                           */
/* Output Argument:                                                          */
/*                                                                           */
/* timeout - output value                                                    */
/*                                                                           */
/* ------------------------------------------------------------------------- */
/* NOTE that program sunmocsv uses binterpapply whereas program sumutecsv    */
/*      uses binterpvalue. The reason is that sunmocsv pre-computes and      */
/*      stores arrays of velocities (a velocity for each sample time).       */
/*      But sumutecsv computes the mute time on-the-fly for the offset       */
/*      from each trace.                                                     */

void binterpvalue(double offset, int mgtextr,
                  double *lwioffs, double *lwitims, int lwinto,
                  double *hiioffs, double *hiitims, int hiinto, 
                  int mgi_tot, double wi, 
                  double *lwcoffs, double *lwctims, int lwcnto,
                  double *hicoffs, double *hictims, int hicnto, 
                  int mgc_tot, double wc,
                  double *timeout) {

  if(mgi_tot==1 && mgc_tot==1) {  
    linterpd(offset,lwioffs,lwitims,lwinto,mgtextr,timeout);
    return;
  }

  *timeout = 0.;
  double time = 0.;

  if(mgc_tot==1) {
    if(wi != 0.) { /* because of extrapolation options, check exactly 0 */
      linterpd(offset,lwioffs,lwitims,lwinto,mgtextr,&time);
      *timeout += wi*time;  
    }
    if(wi != 1.) { /* because of extrapolation options, check exactly 1 */
      linterpd(offset,hiioffs,hiitims,hiinto,mgtextr,&time);
      *timeout += (1.0-wi)*time;  
    }
    return;
  }

  if(mgi_tot==1) { 
    if(wc != 0.) {
      linterpd(offset,lwcoffs,lwctims,lwcnto,mgtextr,&time);
      *timeout += wc*time;  
    }
    if(wc != 1.) {
      linterpd(offset,hicoffs,hictims,hicnto,mgtextr,&time);
      *timeout += (1.0-wc)*time;  
    }
    return;
  }

/* The 4 point weighting equation looks like this:           */  
/*  *timeout =  wc      * (wi*timea + (1.0-wi)*timeb)        */  
/*           + (1.0-wc) * (wi*timec + (1.0-wi)*timed);       */
/*                                                           */
/* But reduce some brackets and it looks like this:          */  
/*  *timeout =  wc*wi*timea + wc*(1.0-wi)*timeb              */  
/*           + (1.0-wc)*wi*timec + (1.0-wc)*(1.0-wi)*timed;  */
/*                                                           */
/* So we can isolate the weight factors needed for each of   */  
/* the 4 locations, as follows:                              */  

  double aw = wc*wi;
  double bw = wc*(1.0-wi);
  double cw = (1.0-wc)*wi;
  double dw = (1.0-wc)*(1.0-wi);

/* Which means we do not have to call linterpd when we know  */  
/* the corresponding weight is zero. This may seem like it   */  
/* will only save a small amount of CPU time but remember    */  
/* that most situations have many cdps outside of the area   */  
/* that is completely surrounded by input mute locations.    */  
/* When outside the surrounded area, the binterpfind routine */  
/* produces wi=0 or 1 and/or wc=0 or 1 if NOT extrapolating. */  

  if(aw != 0.) {
    linterpd(offset,lwioffs,lwitims,lwinto,mgtextr,&time);
    *timeout += aw*time;
  }
  if(bw != 0.) {
    linterpd(offset,hiioffs,hiitims,hiinto,mgtextr,&time);
    *timeout += bw*time;
  }

  if(cw != 0.) {
    linterpd(offset,lwcoffs,lwctims,lwcnto,mgtextr,&time);
    *timeout += cw*time;
  }
  if(dw != 0.) {
    linterpd(offset,hicoffs,hictims,hicnto,mgtextr,&time);
    *timeout += dw*time;
  }

  return;
}
