/* Copyright (c) Colorado School of Mines, 2011.*/
/* All rights reserved.                       */

/* SUSYNLV: $Revision: 1.22 $ ; $Date: 2015/06/02 20:15:23 $	*/

#include "su.h"
#include "segy.h"
#include "synthetics.h"

/*********************** self documentation **********************/
char *sdoc[] = {
"									",
" SUSYNLV - SYNthetic seismograms for Linear Velocity function		",
"									",
" susynlv >outfile [optional parameters]				",
"									",
" Optional Parameters:							",
" nt=101                 number of time samples				",
" dt=0.04                time sampling interval (sec)			",
" ft=0.0                 first time (sec)				",
" kilounits=1            input length units are km or kilo-feet		",
"			 =0 for m or ft					",
"                        Note: Output (sx,gx,offset) are always m or ft ",
" nxo=1                  number of source-receiver offsets		",
" dxo=0.05               offset sampling interval (kilounits)		",
" fxo=0.0                first offset (kilounits, see notes below)	",
" xo=fxo,fxo+dxo,...     array of offsets (use only for non-uniform offsets)",
" nxm=101                number of midpoints (see notes below)		",
" dxm=0.05               midpoint sampling interval (kilounits)		",
" fxm=0.0                first midpoint (kilounits)			",
" nxs=101                number of shotpoints (see notes below)		",
" dxs=0.05               shotpoint sampling interval (kilounits)	",
" fxs=0.0                first shotpoint (kilounits)			",
" x0=0.0                 distance x at which v00 is specified		",
" z0=0.0                 depth z at which v00 is specified		",
" v00=2.0                velocity at x0,z0 (kilounits/sec)		",
" dvdx=0.0               derivative of velocity with distance x (dv/dx)	",
" dvdz=0.0               derivative of velocity with depth z (dv/dz)	",
" fpeak=0.2/dt           peak frequency oftminf symmetric Ricker wavelet (Hz)",
" ref=\"1:1,2;4,2\"        reflector(s):  \"amplitude:x1,z1;x2,z2;x3,z3;...\"",
" smooth=0               =1 for smooth (piecewise cubic spline) reflectors",
" er=0                   =1 for exploding reflector amplitudes		",
" ls=0                   =1 for line source; default is point source	",
" ob=1                   =1 to include obliquity factors		",
" tmin=10.0*dt           minimum time of interest (sec)			",
" ndpfz=5                number of diffractors per Fresnel zone		",
" verbose=0              =1 to print some useful information		",
"									",
"Notes:								",
"Offsets are signed - may be positive or negative.  Receiver locations	",
"are computed by adding the signed offset to the source location.	",
"									",
"Specify either midpoint sampling or shotpoint sampling, but not both.	",
"If neither is specified, the default is the midpoint sampling above.	",
"									",
"More than one ref (reflector) may be specified. Do this by putting	",
"additional ref= entries on the commandline. When obliquity factors	",
"are included, then only the left side of each reflector (as the x,z	",
"reflector coordinates are traversed) is reflecting.  For example, if x	",
"coordinates increase, then the top side of a reflector is reflecting.	",
"Note that reflectors are encoded as quoted strings, with an optional	",
"reflector amplitude: preceding the x,z coordinates of each reflector.	",
"Default amplitude is 1.0 if amplitude: part of the string is omitted.	",
NULL};

/*
 * Credits: CWP Dave Hale, 09/17/91,  Colorado School of Mines
 *	    UTulsa Chris Liner 5/22/03 added kilounits flag
 *
 * Trace header fields set: trid, counit, ns, dt, delrt,
 *				tracl. tracr, fldr, tracf,
 *				cdp, cdpt, d2, f2, offset, sx, gx
 */
/**************** end self doc ***********************************/


/* these structures are defined in par.h -- this is documentation only
 *
 * typedef struct ReflectorSegmentStruct {
 *	float x;	( x coordinate of segment midpoint )
 *	float z;	( z coordinate of segment midpoint )
 *	float s;	( x component of unit-normal-vector )
 *	float c;	( z component of unit-normal-vector )
 * } ReflectorSegment;
 * typedef struct ReflectorStruct {
 *	int ns;			( number of reflector segments )
 *	float ds;		( segment length )
 *	float a;		( amplitude of reflector )
 *	ReflectorSegment *rs;	( array[ns] of reflector segments )
 * } Reflector;
 * typedef struct WaveletStruct {
 *	int lw;			( length of wavelet )
 *	int iw;			( index of first wavelet sample )
 *	float *wv;		( wavelet sample values )
 * } Wavelet;
 *
 */

/* parameters for half-derivative filter */
#define LHD 20
#define NHD 1+2*LHD

/* prototype */
static void makeone (float v00, float dvdx, float dvdz, 
	int ls, int er, int ob, Wavelet *w,
	float xs, float zs, float xg, float zg,
	int nr, Reflector *r, int nt, float dt, float ft, float *trace);


void susynlv_filltrace(
    segy *tr, int shots, int kilounits, int tracl,
    float fxs, int ixsm, float dxs,
    int ixo, float *xo, float dxo, float fxo,
    float dxm, float fxm, float dxsm,
    float v00, float dvdx, float dvdz, int ls, int er, int ob, Wavelet *w,
    int nr, Reflector *r, int nt, float dt, float ft
){
    float xs, zs, xg, zg;
    // print input to make sure they're the same?
    printf("susynlv visible:\n");
    printf("trace info: %d %d %d %d %d\n", tr->trid, tr->counit, tr->ns, tr->dt, tr->delrt);
    printf("1: %d %d %d\n", shots, kilounits, tracl);
    printf("2: %f %d %f\n", fxs, ixsm, dxs);
    printf("3: %d %f %f %f\n", ixo, xo[0], dxo, fxo);
    printf("4: %f %f %f\n", dxm, fxm, dxsm);
    printf("5: %f %f %f\n", v00, dvdx, dvdz);
    printf("6: %d %d %d\n", ls, er, ob);
    printf("7: %d %d %d\n", w->lw, nr, r[0].ns);
    printf("8: %d %f %f\n", nt, dt, ft);

    printf("sizeof %d\n", sizeof(fxs));

    printf("\n");
    /* compute source and receiver coordinates */
    if (shots)
        xs = fxs+ixsm*dxs;
    else
        xs = fxm+ixsm*dxm-0.5*xo[ixo];
    zs = 0.0;
    xg = xs+xo[ixo];
    zg = 0.0;

    /* set segy trace header parameters */
    tr->tracl = tr->tracr = tracl;
    if (shots) {
        tr->fldr = 1+ixsm;
        tr->tracf = 1+ixo;
        tr->d2 = dxo;
        tr->f2 = fxo;
    } else {
        tr->cdp = 1+ixsm;
        tr->cdpt = 1+ixo;
        tr->d2 = dxm;
        tr->f2 = fxm;
    }
    if (kilounits==1) {
        tr->offset = NINT(1000.0*(dxsm>0.0?xo[ixo]:-xo[ixo]));
        tr->sx = NINT(1000.0*xs);
        tr->gx = NINT(1000.0*xg);
    } else {
        tr->offset = NINT((dxsm>0.0?xo[ixo]:-xo[ixo]));
        tr->sx = NINT(xs);
        tr->gx = NINT(xg);
    }

    /* make one trace */
    makeone(v00,dvdx,dvdz,ls,er,ob,w,
        xs,zs,xg,zg,
        nr,r,nt,dt,ft,tr->data);
}


static void makeone (float v00, float dvdx, float dvdz, 
	int ls, int er, int ob, Wavelet *w,
	float xs, float zs, float xg, float zg,
	int nr, Reflector *r, int nt, float dt, float ft, float *trace)
/*****************************************************************************
Make one synthetic seismogram for linear velocity v(x,z) = v00+dvdx*x+dvdz*z
******************************************************************************
Input:
v00		velocity v at (x=0,z=0)
dvdx		derivative dv/dx of velocity v with respect to x
dvdz		derivative dv/dz of velocity v with respect to z
ls		=1 for line source amplitudes; =0 for point source
er		=1 for exploding, =0 for normal reflector amplitudes
ob		=1 to include cos obliquity factors; =0 to omit
w		wavelet to convolve with trace
xs		x coordinate of source
zs		z coordinate of source
xg		x coordinate of receiver group
zg		z coordinate of receiver group
nr		number of reflectors
r		array[nr] of reflectors
nt		number of time samples
dt		time sampling interval
ft		first time sample

Output:
trace		array[nt] containing synthetic seismogram
*****************************************************************************/
{
	int it,ir,is,ns;
	float ar,ds,xd,zd,cd,sd,vs,vg,vd,cs,ss,ts,qs,cg,sg,tg,qg,
		ci,cr,time,amp,*temp;
	ReflectorSegment *rs;
	int lhd=LHD,nhd=NHD;
	static float hd[NHD];
	static int madehd=0;
	
	/* if half-derivative filter not yet made, make it */
	if (!madehd) {
		mkhdiff(dt,lhd,hd);
		madehd = 1;
	}

	/* zero trace */
	for (it=0; it<nt; ++it)
		trace[it] = 0.0;
	
	/* velocities at source and receiver */
	vs = v00+dvdx*xs+dvdz*zs;
	vg = v00+dvdx*xg+dvdz*zg;

	/* loop over reflectors */
	for (ir=0; ir<nr; ++ir) {

		/* amplitude, number of segments, segment length */
		ar = r[ir].a;
		ns = r[ir].ns;
		ds = r[ir].ds;
		rs = r[ir].rs;
	
		/* loop over diffracting segments */
		for (is=0; is<ns; ++is) {
		
			/* diffractor midpoint, unit-normal, and length */
			xd = rs[is].x;
			zd = rs[is].z;
			cd = rs[is].c;
			sd = rs[is].s;
			
			/* velocity at diffractor */
			vd = v00+dvdx*xd+dvdz*zd;

			/* ray from shot to diffractor */
			raylv2(v00,dvdx,dvdz,xs,zs,xd,zd,&cs,&ss,&ts,&qs);

			/* ray from receiver to diffractor */
			raylv2(v00,dvdx,dvdz,xg,zg,xd,zd,&cg,&sg,&tg,&qg);

			/* cosines of incidence and reflection angles */
			if (ob) {
				ci = cd*cs+sd*ss;
				cr = cd*cg+sd*sg;
			} else {
				ci = 1.0;
				cr = 1.0;
			}

			/* if either cosine is negative, skip diffractor */
			if (ci<0.0 || cr<0.0) continue;

			/* two-way time and amplitude */
			time = ts+tg;
			if (er) {
				amp = sqrt(vg*vd/qg);
			} else {
				if (ls)
					amp = sqrt((vs*vd*vd*vg)/(qs*qg));
				else
					amp = sqrt((vs*vd*vd*vg)/
						(qs*qg*(qs+qg)));
			}
			amp *= (ci+cr)*ar*ds;
				
			/* add sinc wavelet to trace */
			addsinc(time,amp,nt,dt,ft,trace);
		}
	}
	
	/* allocate workspace */
	temp = ealloc1float(nt);
	
	/* apply half-derivative filter to trace */
	convolve_cwp(nhd,-lhd,hd,nt,0,trace,nt,0,temp);

	/* convolve wavelet with trace */
	convolve_cwp(w->lw,w->iw,w->wv,nt,0,temp,nt,0,trace);
	
	/* free workspace */
	free1float(temp);
}
