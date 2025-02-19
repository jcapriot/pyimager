/*  #include "segy.h" */

#include "su.h"
#include "gridxy.h"

/* The gridset function must be called before using any other functions herein.      */
/* gridset basically defines a large rectangle containing smaller rectangles (cells).*/
/*                                                                                   */
/* On input to gridset, gvals 2,3,4,5,6,7,10,11 must be specified (these are the     */
/* XYs of corner A, and XYs near what will be corner B, and XYs that will determine  */
/* where corner C is. And the cell widths you want in A-->B and A-->C directions).   */
/*                                                                                   */
/* gridset uses the 8 values above in the following way:                             */
/* Corner A coordinates are used exactly as input.                                   */ 
/* The direction from corner A to corner B is determined by A-->B coordinates.       */ 
/* But then corner B coordinates are adjusted to an exact multiple of cell width WB. */ 
/* Input coordinates of A and C are used to compute the distance from A to C.        */ 
/* Right angle to A-->exactB gives direction for corner C (along line thru A).       */ 
/* Corner C is adjusted to an exact multiple of cell width WC away from A.           */ 
/* Note that this means input corner C XYs are only used to decide how wide the      */
/* grid is, and which side of A-->B is output corner C located on.                   */
/* Finally, corner D is computed just so users can see it.                           */
/*                                                                                   */
/* After gridset, gvals contains the following processed grid definition values:     */
/*                                                                                   */
/* Element     Name      Definition                                                  */
/* -------     ----      ----------                                                  */
/*                                                                                   */
/* gvals[0]  = bintype number (which has nothing to do with grid definition itself)  */
/* gvals[1]  = grid_lf = which side of A-->B is C on? 1=left, -1=right               */
/* gvals[2]  = grid_xa = raw, real world X coordinate of corner A                    */
/* gvals[3]  = grid_ya = raw, real world Y coordinate of corner A                    */
/* gvals[4]  = grid_xb = raw, real world X coordinate of corner B                    */
/* gvals[5]  = grid_yb = raw, real world Y coordinate of corner B                    */
/* gvals[6]  = grid_xc = raw, real world X coordinate of corner C                    */
/* gvals[7]  = grid_yc = raw, real world Y coordinate of corner C                    */
/* gvals[8]  = grid_xd = raw, real world X coordinate of corner D                    */
/* gvals[9]  = grid_yd = raw, real world Y coordinate of corner D                    */
/* gvals[10] = grid_wb = width of cells in A-->B direction                           */
/* gvals[11] = grid_wc = width of cells in A-->C direction                           */
/* gvals[12] = grid_nb = number of cells in A-->B direction                          */
/* gvals[13] = grid_nc = number of cells in A-->C direction                          */
/* gvals[14] = grid_fp = first cdp (cell) number                                     */
/* gvals[15] = grid_lp = last  cdp (cell) number                                     */
/* gvals[16] = grid_sb = sine   of A-->B to X-axis.                                  */
/* gvals[17] = grid_cb = cosine of A-->B to X-axis.                                  */
/*                                                                                   */
/*                                                                                   */
/* On return:                                                                        */
/*  errwarn > 0 means grid definition is not usable.                                 */
/*            1 means grid_wb cell width was <= 0.0                                  */
/*            2 means grid_wc cell width was <= 0.0                                  */
/*            3 means corner B is within grid_wb cell width of corner A.             */
/*           -1 means corner C near A and reset to A (grid only has 1 cell sideways) */
/*                                                                                   */
/* Note: In the Object Orientated paradigm, the processed grid definition values     */
/*       would all be hidden (in C++ they would be in private variables and only     */
/*       set-able using private methods). And so on. But SU is supposed to be        */
/*       simple, and a learning tool, so I have left everything exposed.             */
/*       If you change gvals after gridset, woe be to you.                           */

void gridset(double *gvals, int *errwarn) { 

  *errwarn = 0;

  if(gvals[10] <= 0.0) {
    *errwarn = 1;
    return;
  }
  if(gvals[11] <= 0.0) {
    *errwarn = 2;
    return;
  }

/* Reset corner B to be at an exact multiple distance of the B cell width.       */
/* Do not want compilors to optimize these computations, so use explicit (int).  */

  double dab = sqrt((gvals[2] - gvals[4])*(gvals[2] - gvals[4]) 
                  + (gvals[3] - gvals[5])*(gvals[3] - gvals[5])); 

  int nwb = (int) (dab/gvals[10] + 0.5); 

  double dabwb = nwb * gvals[10];

  if(nwb<1) {
    *errwarn = 3;
    return;
  }

  nwb++; /* reset from number of intervals to number of cells */

  gvals[4] = gvals[2] + dabwb/dab * (gvals[4] - gvals[2]);
  gvals[5] = gvals[3] + dabwb/dab * (gvals[5] - gvals[3]);

/* Compute the input distance from A-->C.                                        */
/* And the exact multiple distance of the C cell width.                          */

  double dac = sqrt((gvals[2] - gvals[6])*(gvals[2] - gvals[6]) 
                  + (gvals[3] - gvals[7])*(gvals[3] - gvals[7])); 

  int nwc = (int) (dac/gvals[11] + 0.5); 

  if(nwc<1) *errwarn = -1; /*  Corner C is near A and is reset to A */

  nwc++; /* reset from number of intervals to number of cells */

  gvals[12] = nwb; /* set number of cells in A-->B direction */
  gvals[13] = nwc; /* set number of cells in A-->C direction */
  gvals[14] = 1;   /* set first cdp number (may allow different, eventually) */
  gvals[15] = gvals[14] + nwb*nwc - 1; /* set last cdp number */
  gvals[16] = (gvals[5] - gvals[3]) / dabwb; /* set sine   */
  gvals[17] = (gvals[4] - gvals[2]) / dabwb; /* set cosine */

/* Determine what side of A-->B the input C point is on.                         */

  double dx = gvals[6] - gvals[2];
  double dy = gvals[7] - gvals[3];

/*double tx =  dx*gvals[17] + dy*gvals[16];  do not need x value */
  double ty = -dx*gvals[16] + dy*gvals[17];

  gvals[1] = 1.;
  if(ty<0.) gvals[1] = -1.; /* set leftright coordinate multiplier */

/* Reset/set coordinates of corners B,C,D to values computed by gridicrawxy */
/* (sine,cosine were determined by floating point computations and we want  */
/*  the corners to be what gridicrawxy produces in case user chooses to use */
/*  the corner coordinates as range-limits and so on).                      */

  double rx;
  double ry;

  gridicrawxy(gvals,nwb,1,&rx,&ry);   /* get corner B XYs */
  gvals[4] = rx;
  gvals[5] = ry;

  gridicrawxy(gvals,1,nwc,&rx,&ry);   /* get corner C XYs */
  gvals[6] = rx;
  gvals[7] = ry;

  gridicrawxy(gvals,nwb,nwc,&rx,&ry); /* get corner D XYs */
  gvals[8] = rx;
  gvals[9] = ry;

}    

void gridrawxycdpic(double *gvals,double dx,double dy,int *icdp,int *igi,int *igc) {

/* Convert raw (real world) coordinates to cdp and igi,igc indexes.    */
/*                                                                     */
/* Inputs:                                                             */
/*   gvals is grid definition after processing by gridset              */
/*   dx    is x coordinate (raw,real world, after scalco applied)      */
/*   dy    is y coordinate (raw,real world, after scalco applied)      */
/* Outputs:                                                            */
/*   icdp is computed cell number. If input X,Y are outside the grid   */
/*        then icdp is output as -2147483645 but all other output      */
/*        values are still properly computed. So igi,igc will be less  */
/*        than 1 or greater than number of cells in their direction.   */
/*        And rx,ry will be negative or greater than grid extent.      */
/*        Note that rx,ry can be negative anyway when icdp is good     */
/*        since corners are at the centre of cells.                    */
/*   igi  is computed cell index in A-->B direction (first igi is 1).  */
/*   igc  is computed cell index in A-->C direction (first igc is 1).  */

  dx = dx - gvals[2];
  dy = dy - gvals[3];

/* careful here, we want to rotate back to the axis from raw */

  double rx =   dx*gvals[17] + dy*gvals[16]; 
  double ry = (-dx*gvals[16] + dy*gvals[17]) * gvals[1];

/* Compute the cell index number in b and c directions.     */
/* Add 0.5 because corner A is at the centre of first cell. */
/* Add 1 to start indexes at 1 (more user-friendly).        */

  *igi = (int) floor((rx / gvals[10] + 1.5)); 
  *igc = (int) floor((ry / gvals[11] + 1.5)); 

/* Compute the cell cdp number.                             */

  if(*igi<1 || *igi>gvals[12] || *igc<1 || *igc>gvals[13]) {
    *icdp = -2147483645;
  }
  else { 
    *icdp = gvals[14] + *igi-1 + (*igc-1) * gvals[12];
  }
}

void gridrawxygridxy(double *gvals,double dx,double dy,double *tx,double *ty) {

/* Convert raw (real world) coordinates to grid coordinates.           */
/*                                                                     */
/* Inputs:                                                             */
/*   gvals is grid definition after processing by gridset              */
/*   dx    is x coordinate (raw,real world, after scalco applied)      */
/*   dy    is y coordinate (raw,real world, after scalco applied)      */
/* Outputs:                                                            */
/*   tx    is x coordinate within grid.                                */
/*   ty    is y coordinate within grid.                                */
/*                                                                     */
/* Transforming from raw,real world XYs to grid XYs involves:          */
/*  - subtracting cornerA coordinates                                  */
/*  - rotating to cornerA-->B direction using grid sin,cosine          */
/*  - mirroring (multiplying gridY by -1 if C is on right of A-->B)    */
/*                                                                     */
/* Note: this function does not care if dx,dy are outside grid,        */
/*       it returns tx,ty anyway.                                      */

  dx = dx - gvals[2];
  dy = dy - gvals[3]; 

  *tx =   dx*gvals[17] + dy*gvals[16]; 
  *ty = (-dx*gvals[16] + dy*gvals[17]) * gvals[1];;

}


void gridgridxyrawxy(double *gvals,double dx,double dy,double *tx,double *ty) {

/* Convert grid coordinates to raw (real world) coordinates            */
/*                                                                     */
/* Inputs:                                                             */
/*   gvals is grid definition after processing by gridset              */
/*   dx    is x in grid coordiantes                                    */
/*   dy    is y in grid coordinates                                    */
/* Outputs:                                                            */
/*   tx    is x raw (real world) coordinate                            */
/*   ty    is y raw (real world) coordinate                            */
/*                                                                     */
/* Transforming from grid XYs to raw,real world XYs involves:          */
/*  - mirroring (multiplying gridY by -1 if C is on right of A-->B)    */
/*  - rotating from cornerA-->B direction using grid sin,cosine        */
/*  - adding cornerA coordinates                                       */
/*                                                                     */
/* Note: this function does not care if dx,dy are outside grid,        */
/*       it returns tx,ty anyway.                                      */

   dy = dy * gvals[1];

  *tx = dx*gvals[17] - dy*gvals[16];             
  *ty = dx*gvals[16] + dy*gvals[17];

  *tx = *tx + gvals[2];
  *ty = *ty + gvals[3];

}

void gridicgridxy(double *gvals,int igi,int igc,double *dx,double *dy) {

/* Convert grid indexes igi,igc to cell centre in grid XYs.            */
/*                                                                     */
/* Inputs:                                                             */
/*   gvals is grid definition after processing by gridset              */
/*   igi  is computed cell index in A-->B direction (first igi is 1).  */
/*   igc  is computed cell index in A-->C direction (first igc is 1).  */
/* Outputs:                                                            */
/*   dx   is cell centre X (in grid coordinates)                       */
/*   dy   is cell centre Y (in grid coordinates)                       */
/*                                                                     */
/* Note: this function does not care if igi or igc are outside grid,   */
/*       it returns the XYs anyway.                                    */

  *dx = (igi-1)*gvals[10]; 
  *dy = (igc-1)*gvals[11]; 

}

void gridicrawxy(double *gvals,int igi,int igc,double *dx,double *dy) {

/* Convert grid indexes igi,igc to cell centre in raw (real world) XYs.*/
/*                                                                     */
/* Inputs:                                                             */
/*   gvals is grid definition after processing by gridset              */
/*   igi  is computed cell index in A-->B direction (first igi is 1).  */
/*   igc  is computed cell index in A-->C direction (first igc is 1).  */
/* Outputs:                                                            */
/*   dx   is cell centre X (in raw, real world coordinates)            */
/*   dy   is cell centre Y (in raw, real world coordinates)            */
/*                                                                     */
/* Transforming from grid XYs to raw,real world XYs involves:          */
/*  - mirroring (multiplying gridY by -1 if C is on right of A-->B)    */
/*  - rotating from cornerA-->B direction using grid sin,cosine        */
/*  - adding cornerA coordinates                                       */
/*                                                                     */
/* Note: this function does not care if igi or igc are outside grid,   */
/*       it returns the XYs anyway.                                    */

  double rx = (igi-1)*gvals[10]; 
  double ry = (igc-1)*gvals[11] * gvals[1]; 
       
  *dx = rx*gvals[17] - ry*gvals[16];
  *dy = rx*gvals[16] + ry*gvals[17];

  *dx = *dx + gvals[2];
  *dy = *dy + gvals[3];

}

void gridiccdp(double *gvals,int igi,int igc,int *icdp) {

/* Convert grid indexes igi,igc to cdp (cell) number.                  */
/*                                                                     */
/* Inputs:                                                             */
/*   gvals is grid definition after processing by gridset              */
/*   igi  is computed cell index in A-->B direction (first igi is 1).  */
/*   igc  is computed cell index in A-->C direction (first igc is 1).  */
/* Outputs:                                                            */
/*   icdp is cell cdp number                                           */
/* Note: if igi or igc are outside grid, icdp=-2147483645 is returned  */

  if(igi<1 || igi>gvals[12] || igc<1 || igc>gvals[13]) {
    *icdp = -2147483645;
  }
  else { 
    *icdp = gvals[14] + igi-1 + (igc-1) * gvals[12];
  }

}

void gridiccdp90(double *gvals,int igi,int igc,int *jcdp) {

/* Convert grid indexes igi,igc to 90 degree cdp (cell) number         */
/*                                                                     */
/* Inputs:                                                             */
/*   gvals is grid definition after processing by gridset              */
/*   igi  is computed cell index in A-->B direction (first igi is 1).  */
/*   igc  is computed cell index in A-->C direction (first igc is 1).  */
/* Outputs:                                                            */
/*   jcdp is cell cdp numbered in direction of corner C rather than B. */
/* Note: if igi or igc are outside grid, icdp=-2147483645 is returned  */
/* Note: igi and igc numbers are not rotated, just the same as always. */

  if(igi<1 || igi>gvals[12] || igc<1 || igc>gvals[13]) {
    *jcdp = -2147483645;
  }
  else { 
    *jcdp = gvals[14] + (igi-1) * gvals[13] + igc-1;
  }

}

void gridcdpic(double *gvals,int icdp,int *igi,int *igc) {

/* Convert cdp (cell) number to grid indexes igi,igc.                  */
/*                                                                     */
/* Inputs:                                                             */
/*   gvals is grid definition after processing by gridset              */
/*   icdp is cell cdp number                                           */
/* Outputs:                                                            */
/*   igi  is computed cell index in A-->B direction (first igi is 1).  */
/*   igc  is computed cell index in A-->C direction (first igc is 1).  */
/* Note: if icdp is outside grid, igi,igc returned as -2147483645      */

  if(icdp<gvals[14] || icdp>gvals[15]) {
    *igi = -2147483645;
    *igc = -2147483645;
    return;
  }

  int ncdp = icdp - gvals[14];
  int nwb  = gvals[12];

  *igi = 1 + ncdp%nwb;
  *igc = 1 + ncdp/nwb;

}

void gridcdpic90(double *gvals,int jcdp,int *igi,int *igc) {

/* Convert cdp (cell) number to grid indexes igi,igc.                  */
/*                                                                     */
/* Inputs:                                                             */
/*   gvals is grid definition after processing by gridset              */
/*   jcdp is cell cdp numbered in corner C direction, rather than B.   */
/* Outputs:                                                            */
/*   igi  is computed cell index in A-->B direction (first igi is 1).  */
/*   igc  is computed cell index in A-->C direction (first igc is 1).  */
/* Note: if jcdp is outside grid, igi,igc returned as -2147483645      */
/* Note: igi and igc numbers are not rotated, just the same as always. */

  if(jcdp<gvals[14] || jcdp>gvals[15]) {
    *igi = -2147483645;
    *igc = -2147483645;
    return;
  }

  int ncdp = jcdp - gvals[14];
  int nwc  = gvals[13];

  *igi = 1 + ncdp/nwc;
  *igc = 1 + ncdp%nwc;

}


void gridcheck(double *gvals, int icheck, int *errwarn) { 

/* Exercise grid functions using the 4 coorners.                       */
/*                                                                     */
/* Inputs:                                                             */
/*   gvals is grid definition after processing by gridset              */
/*   icheck=0 is no checking.                                          */
/*            else get the corner XYs from gvals and run them through  */
/*            various grid functions and print the results.            */
/*                                                                     */
/* The basic idea is to check sin,cosine and leftright coding errors.  */
/* But users can also make use of the results if they are confused as  */
/* to what is going on. For extensive testing, start with a simple     */
/* situation where A-->B is roughtly aligned with X-axis with corner B */
/* having a larger X coordinate than A. And put corner C on left side  */
/* near corner A (a larger Y coordinate than A). Then put C on right   */
/* side of A. After that, use the output wfile and exchange A and B    */
/* and C and D (for both the previous runs). All these kinds of tests  */
/* should result in cdp,igi,igc making sense (not negative and so on). */
/* Similarly, the raw-to-grid and grid-to-raw coordinate conversion    */
/* functions should produce understandable results.                    */
/*                                                                     */
/* Outputs:                                                            */
/*   errwarn (always 0 in this version).                               */

  *errwarn = 0;
  if(icheck==0) return;

  int nwb = gvals[12] + 0.1;
  int nwc = gvals[13] + 0.1;
  double rx;
  double ry;
  double tx;
  double ty;
  int jcdp;
  int jigi;
  int jigc;
  int kcdp;
  int kigi;
  int kigc;

  gridicrawxy(gvals,1,1,&rx,&ry);   
  warn("gridicrawxy:     corner A raw  XYs= %.20f %.20f ",rx,ry);
  gridrawxygridxy(gvals,rx,ry,&tx,&ty);  
  warn("gridrawxygridxy: corner A grid XYs= %.20f %.20f ",tx,ty);
  gridicgridxy(gvals,1,1,&tx,&ty);   
  warn("gridicgridxy:    corner A grid XYs= %.20f %.20f ",tx,ty);
  gridgridxyrawxy(gvals,tx,ty,&rx,&ry);   
  warn("gridgridxyrawxy: corner A raw  XYs= %.20f %.20f ",rx,ry);
  gridrawxycdpic(gvals,rx,ry,&jcdp,&jigi,&jigc); 
  gridcdpic(gvals,jcdp,&kigi,&kigc);
  gridiccdp(gvals,jigi,jigc,&kcdp); 
  warn("gridrawxycdpic:          corner A cdp,igi,igc = %d %d %d ",jcdp,jigi,jigc);
  warn("gridcdpic and gridiccdp: corner A cdp,igi,igc = %d %d %d ",kcdp,kigi,kigc);


  gridicrawxy(gvals,nwb,1,&rx,&ry);   /* get corner B XYs */
  warn("gridicrawxy:     corner B raw  XYs= %.20f %.20f ",rx,ry);
  gridrawxygridxy(gvals,rx,ry,&tx,&ty);   /* get grid B XYs */
  warn("gridrawxygridxy: corner B grid XYs= %.20f %.20f ",tx,ty);
  gridicgridxy(gvals,nwb,1,&tx,&ty);   /* get grid B XYs */
  warn("gridicgridxy:    corner B grid XYs= %.20f %.20f ",tx,ty);
  gridgridxyrawxy(gvals,tx,ty,&rx,&ry);   /* get corner B XYs */
  warn("gridgridxyrawxy: corner B raw  XYs= %.20f %.20f ",rx,ry);
  gridrawxycdpic(gvals,rx,ry,&jcdp,&jigi,&jigc); /* get icdp,igi,igc from corner A XYs */
  gridcdpic(gvals,jcdp,&kigi,&kigc); /* get igi,igc from cdp */
  gridiccdp(gvals,jigi,jigc,&kcdp); /* get cpd from igi,igc */
  warn("gridrawxycdpic:          corner B cdp,igi,igc = %d %d %d ",jcdp,jigi,jigc);
  warn("gridcdpic and gridiccdp: corner B cdp,igi,igc = %d %d %d ",kcdp,kigi,kigc);


  gridicrawxy(gvals,1,nwc,&rx,&ry);   /* get corner C XYs */
  warn("gridicrawxy:     corner C raw  XYs= %.20f %.20f ",rx,ry);
  gridrawxygridxy(gvals,rx,ry,&tx,&ty);   /* get grid C XYs */
  warn("gridrawxygridxy: corner C grid XYs= %.20f %.20f ",tx,ty);
  gridicgridxy(gvals,1,nwc,&tx,&ty);   /* get grid C XYs */
  warn("gridicgridxy:    corner C grid XYs= %.20f %.20f ",tx,ty);
  gridgridxyrawxy(gvals,tx,ty,&rx,&ry);   /* get corner C XYs */
  warn("gridgridxyrawxy: corner C raw  XYs= %.20f %.20f ",rx,ry);
  gridrawxycdpic(gvals,rx,ry,&jcdp,&jigi,&jigc); /* get icdp,igi,igc from corner A XYs */
  gridcdpic(gvals,jcdp,&kigi,&kigc); /* get igi,igc from cdp */
  gridiccdp(gvals,jigi,jigc,&kcdp); /* get cpd from igi,igc */
  warn("gridrawxycdpic:          corner C cdp,igi,igc = %d %d %d ",jcdp,jigi,jigc);
  warn("gridcdpic and gridiccdp: corner C cdp,igi,igc = %d %d %d ",kcdp,kigi,kigc);


  gridicrawxy(gvals,nwb,nwc,&rx,&ry); /* get corner D XYs */
  warn("gridicrawxy:     corner D raw  XYs= %.20f %.20f ",rx,ry);
  gridrawxygridxy(gvals,rx,ry,&tx,&ty);   /* get grid D XYs */
  warn("gridrawxygridxy: corner D grid XYs= %.20f %.20f ",tx,ty);
  gridicgridxy(gvals,nwb,nwc,&tx,&ty); /* get grid D XYs */
  warn("gridicgridxy:    corner D grid XYs= %.20f %.20f ",tx,ty);
  gridgridxyrawxy(gvals,tx,ty,&rx,&ry);   /* get corner D XYs */
  warn("gridgridxyrawxy: corner D raw  XYs= %.20f %.20f ",rx,ry);
  gridrawxycdpic(gvals,rx,ry,&jcdp,&jigi,&jigc); /* get icdp,igi,igc from corner A XYs */
  gridcdpic(gvals,jcdp,&kigi,&kigc); /* get igi,igc from cdp */
  gridiccdp(gvals,jigi,jigc,&kcdp); /* get cpd from igi,igc */
  warn("gridrawxycdpic:          corner D cdp,igi,igc = %d %d %d ",jcdp,jigi,jigc);
  warn("gridcdpic and gridiccdp: corner D cdp,igi,igc = %d %d %d ",kcdp,kigi,kigc);

}    
