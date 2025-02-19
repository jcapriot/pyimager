/* Copyright (c) Colorado School of Mines, 2021.*/
/* All rights reserved.                       */

/* GRIDXY.H: $Revision: 1.1 $ ; $Date: 2021/10/03 23:17:54 $        */

#ifndef GRIDXY_H
#define GRIDXY_H

void gridset(double *gvals, int *errwarn) ; 
void gridrawxycdpic(double *gvals,double dx,double dy,int *icdp,int *igi,int *igc) ;
void gridicrawxy(double *gvals,int igi,int igc,double *dx,double *dy) ;
void gridicgridxy(double *gvals,int igi,int igc,double *dx,double *dy) ;
void gridiccdp(double *gvals,int igi,int igc,int *icdp) ;
void gridiccdp90(double *gvals,int igi,int igc,int *jcdp) ;
void gridcdpic(double *gvals,int icdp,int *igi,int *igc) ;
void gridcdpic90(double *gvals,int jcdp,int *igi,int *igc) ;
void gridrawxygridxy(double *gvals,double dx,double dy,double *tx,double *ty) ;
void gridgridxyrawxy(double *gvals,double dx,double dy,double *tx,double *ty) ;
void gridcheck(double *gvals, int icheck, int *errwarn) ; 

#endif /* end GRIDXY_H */
