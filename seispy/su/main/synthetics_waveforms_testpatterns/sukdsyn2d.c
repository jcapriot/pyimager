#include "su.h"
#include "segy.h"
#include "synthetics.h"

/* prototypes of functions used internally */
void integ(float **mig,int nz,float dz,int nx,int m,float **migi);
void resit(int nx,float fx,float dx,int nz,int nr,float dr,
	float **tb,float **t,float x0);
void sum2(int nx,int nz,float a1,float a2,float **t1,float **t2,float **t);
void timeb(int nr,int nz,float dr,float dz,float fz,float a,
	float v0,float **t,float **p,float **sigb,float **cosb);
void maketrace(float *trace,int nt,float ft,float dt,
	float xs,float xg,float **mig,float **migi,float aperx,
  	int nx,float fx,float dx,int nz,float fz,float dz,
	int mzmax,int ls,float angmax,float v0,float fmax,Wavelet *w,
	float **tb,float **pb,float **sigb,float **cosb,int nr,float **tsum,
	int nxt,float fxt,float dxt,int nzt,float fzt,float dzt);

/* parameters for half-derivative filter */
#define LHD 20
#define NHD 1+2*LHD

// get and increment tracl
void kdsyn2d_filltrace(
    segy *tr, int tracl, int ixs, int ixo, float xo, float xs, float ds, float fs,
    int nxt, int nzt, float **ttab, float **tsum, float **tt
){
    float as,res;
    int is;
    float xg = xs+xo;
    /* set segy trace header parameters */
    tr->tracl =tr->tracr = tracl
    tr->fldr = 1+ixs;
    tr->tracf = 1+ixo;
    tr->offset = NINT(xo);
    tr->sx = NINT(xs);
    tr->gx = NINT(xg);

    as = (xs-fs)/ds;
    is = (int)as;
    if(is==ns-1) is=ns-2;
    res = as-is;
    if(res<=0.01) res = 0.0;
    if(res>=0.99) res = 1.0;
    sum2(nxt,nzt,1-res,res,ttab[is],ttab[is+1],tsum);

    as = (xg-fs)/ds;
    is = (int)as;
    if(is==ns-1) is=ns-2;
    res = as-is;
    if(res<=0.01) res = 0.0;
    if(res>=0.99) res = 1.0;
    sum2(nxt,nzt,1-res,res,ttab[is],ttab[is+1],tt);
    sum2(nxt,nzt,1,1,tt,tsum,tsum)

    /* make one trace */
    maketrace(tr->data,nt,ft,dt,xs,xg,mig,migi,aperx,
      nx,fx,dx,nz,fz,dz,mzmax,ls,angmax,v0,fmax,w,
      tb,pb,sigb,cosb,nr,tsum,nxt,fxt,dxt,nzt,fzt,dzt);

    /* write trace */
}


void integ(float **mig,int nz,float dz,int nx,int m,float **migi)
/* integration of a two-dimensional array
  input:
    mig[nx][nz]		two-dimensional array
  output:
    migi[nx][nz+2*m] 	integrated array
*/
{
	int nfft, nw, ix, iz, iw;
	float  *amp, dw, *rt;
	complex *ct;


        /* Set up FFT parameters */
        nfft = npfaro(nz+m, 2 * (nz+m));
        if (nfft >= SU_NFLTS || nfft >= 720720)
                err("Padded nt=%d -- too big", nfft);

        nw = nfft/2 + 1;
	dw = 2.0*PI/(nfft*dz);

	amp = ealloc1float(nw);
	for(iw=1; iw<nw; ++iw)
		amp[iw] = 0.5/(nfft*(1-cos(iw*dw*dz)));
	amp[0] = amp[1];

        /* Allocate fft arrays */
        rt   = ealloc1float(nfft);
        ct   = ealloc1complex(nw);

	for(ix=0; ix<nx; ++ix) {
        	memcpy(rt, mig[ix], nz*FSIZE);
       	 	memset((void *) (rt + nz), 0, (nfft-nz)*FSIZE);
        	pfarc(1, nfft, rt, ct);

        	/* Integrate traces   */
		for(iw=0; iw<nw; ++iw){
			ct[iw].i = ct[iw].i*amp[iw];
			ct[iw].r = ct[iw].r*amp[iw];
		}

        	pfacr(-1, nfft, ct, rt);

        	for (iz=0; iz<m; ++iz)  migi[ix][iz] = rt[nfft-m+iz];
        	for (iz=0; iz<nz+m; ++iz)  migi[ix][iz+m] = rt[iz];
	}

	free1float(amp);
	free1float(rt);
	free1complex(ct);
}


/* residual traveltime calculation based  on reference   time	*/
  void resit(int nx,float fx,float dx,int nz,int nr,float dr,
		float **tb,float **t,float x0)
{
	int ix,iz,jr;
	float xi,ar,sr,sr0;

	for(ix=0; ix<nx; ++ix){
		xi = fx+ix*dx-x0;
		ar = fabsf(xi)/dr;
		jr = (int)ar;
		sr = ar-jr;
		sr0 = 1.0-sr;
		if(jr>nr-2) jr = nr-2;
		for(iz=0; iz<nz; ++iz)
			t[ix][iz] -= sr0*tb[jr][iz]+sr*tb[jr+1][iz];
	}
}

/* sum of two tables	*/
  void sum2(int nx,int nz,float a1,float a2,float **t1,float **t2,float **t)
{
	int ix,iz;

	for(ix=0; ix<nx; ++ix)
		for(iz=0; iz<nz; ++iz)
			t[ix][iz] = a1*t1[ix][iz]+a2*t2[ix][iz];
}

/* compute  reference traveltime and slowness	*/
      void timeb(int nr,int nz,float dr,float dz,float fz,float a,
	float v0,float **t,float **p,float **sig,float **cs)
{
	int  ir,iz;
	float r,z,v,rc,oa,rou,zc;


	if( a==0.0) {
		for(ir=0,r=0;ir<nr;++ir,r+=dr)
			for(iz=0,z=fz;iz<nz;++iz,z+=dz){
				rou = sqrt(r*r+z*z);
				t[ir][iz] = rou/v0;
				if(rou<dz) rou = dz;
				t[ir][iz] = rou/v0;
				p[ir][iz] = r/(rou*v0);
				sig[ir][iz] = v0*rou;
				cs[ir][iz] = z/rou;
			}
	} else {
		oa = 1.0/a; 	zc = v0*oa;
		for(ir=0,r=0;ir<nr;++ir,r+=dr)
			for(iz=0,z=fz+zc;iz<nz;++iz,z+=dz){
				rou = sqrt(r*r+z*z);
				v = v0+a*(z-zc);
				if(ir==0){
					t[ir][iz] = log(v/v0)*oa;
					p[ir][iz] = 0.0;
					sig[ir][iz] = 0.5*(z-zc)*(v0+v);
					cs[ir][iz] = 1.0;
				} else {
					rc = (r*r+z*z-zc*zc)/(2.0*r);
					rou = sqrt(zc*zc+rc*rc);
					t[ir][iz] = log((v*(rou+rc))
						/(v0*(rou+rc-r)))*oa;
					p[ir][iz] = sqrt(rou*rou-rc*rc)
						/(rou*v0);
				   	cs[ir][iz] = (rc-r)/rou;
					sig[ir][iz] = a*rou*r;
				}
			}
	}
}


void maketrace(float *trace,int nt,float ft,float dt,
	float xs,float xg,float **mig,float **migi,float aperx,
  	int nx,float fx,float dx,int nz,float fz,float dz,
	int mzmax,int ls,float angmax,float v0,float fmax,Wavelet *w,
	float **tb,float **pb,float **sigb,float **cosb,int nr,float **tsum,
	int nxt,float fxt,float dxt,int nzt,float fzt,float dzt)
/*****************************************************************************
Make one synthetic seismogram
******************************************************************************
Input:
**mig		migration section
**migi		integrated migration section
nt		number of time samples in seismic trace
ft		first time sample of seismic trace
dt		time sampleing interval in seismic trace
xs,xg		lateral coordinates of source and geophone
aperx		lateral aperature in migration
nx,fx,dx,nz,fz,dz	dimension parameters of migration region
mzmax		number of depth samples in triangle filter
ls		=1 for line source; =0 for point source
w		wavelet to convolve with trace
angmax		migration angle aperature from vertical
tb,pb,sigb,cosb		reference traveltime, lateral slowness, sigma
		and cosine of emergent angle
nr		number of lateral samples in reference quantities
tsum		sum of residual traveltimes from shot and receiver
nxt,fxt,dxt,nzt,fzt,dzt		dimension parameters of traveltime table

Output:
trace		array[nt] containing synthetic seismogram
*****************************************************************************/
{
	int nxf,nxe,nxtf,nxte,ix,iz,iz0,izt0,nzp,jrs,jrg,jz,jt,jx,mz,iz1;
	float xm,x,dis,rxz,ar,srs,srg,srs0,srg0,sigp,z0,rdz,ampd,
	      sigs,sigg,coss,cosg,ax,ax0,pmin,
	      odt=1.0/dt,pd,az,sz,sz0,at,td,res,temp;
	float *zpt,**ampt,**ampti,**zmt,*amp,*ampi,*zm,*tzt,*work1;
	int lhd=LHD,nhd=NHD;
	static float hd[NHD];
	static int madehd=0;

	/* if half-derivative filter not yet made, make it */
	if (!madehd) {
		mkhdiff(dt,lhd,hd);
		madehd = 1;
	}

	/* zero trace */
	for (jt=0; jt<nt; ++jt)
		trace[jt] = 0.0;

	zmt = ealloc2float(nzt,nxt);
	ampt = ealloc2float(nzt,nxt);
	ampti = ealloc2float(nzt,nxt);
	amp = ealloc1float(nzt);
	ampi = ealloc1float(nzt);
	zm = ealloc1float(nzt);
	tzt = ealloc1float(nzt);
	zpt = ealloc1float(nxt);
	work1 = ealloc1float(nt);

	z0 = (fz-fzt)/dzt;
	pmin = 1.0/(2.0*dx*fmax);
	rdz = dz/dzt;

	xm = 0.5*(xs+xg);
	rxz = (angmax==90)?0.0:1.0/tan(angmax*PI/180.);
	nxtf = (xm-aperx-fxt)/dxt;
	if(nxtf<0) nxtf = 0;
	nxte = (xm+aperx-fxt)/dxt+1.0;
	if(nxte>=nxt) nxte = nxt-1;

	/* compute amplitudes 	*/
	for(ix=nxtf; ix<=nxte; ++ix){
		x = fxt+ix*dxt;
		dis = (xm>=x)?xm-x:x-xm;
		izt0 = ((dis-dxt)*rxz-fzt)/dzt-1;
		if(izt0<0) izt0 = 0;
		if(izt0>=nzt) izt0 = nzt-1;

		ar = (xs>=x)?(xs-x)/dx:(x-xs)/dx;
		jrs = (int)ar;
		if(jrs>nr-2) jrs = nr-2;
		srs = ar-jrs;
		srs0 = 1.0-srs;
		ar = (xg>=x)?(xg-x)/dx:(x-xg)/dx;
		jrg = (int)ar;
		if(jrg>nr-2) jrg = nr-2;
		srg = ar-jrg;
		srg0 = 1.0-srg;
		sigp = ((xs-x)*(xg-x)>0)?1.0:-1.0;
		zpt[ix] = fzt+(nzt-1)*dzt;

		for(iz=izt0; iz<nzt; ++iz){
			sigs = srs0*sigb[jrs][iz]+srs*sigb[jrs+1][iz];
			sigg = srg0*sigb[jrg][iz]+srg*sigb[jrg+1][iz];
			coss = srs0*cosb[jrs][iz]+srs*cosb[jrs+1][iz];
			cosg = srg0*cosb[jrg][iz]+srg*cosb[jrg+1][iz];
			ampd = v0*dx*(coss+cosg)*dz;
			if(ampd<0.0) ampd = -ampd;
			if(ls)
			    ampt[ix][iz] = ampd/sqrt(v0*sigs*sigg);
			else
			    ampt[ix][iz] = ampd/sqrt(sigs*sigg*(sigs+sigg));

			pd = srs0*pb[jrs][iz]+srs*pb[jrs+1][iz]+sigp
			     *(srg0*pb[jrg][iz]+srg*pb[jrg+1][iz]);
			if(pd<0.0) pd = -pd;
			if(pd<0.0) pd = -pd;
			temp = 0.5*pd*v0*dx/dz;
			if(temp<1) temp = 1.0;
			if(temp>mzmax) temp = mzmax;
			ampti[ix][iz] = ampt[ix][iz]/(temp*temp);
			zmt[ix][iz] = temp;
			if(pd<pmin && zpt[ix]>fzt+(nzt-1.1)*dzt)
				zpt[ix] = fzt+iz*dzt;

		}
	}

	nxf = (xm-aperx-fx)/dx+0.5;
	if(nxf<0) nxf = 0;
	nxe = (xm+aperx-fx)/dx+0.5;
	if(nxe>=nx) nxe = nx-1;

	/* interpolate amplitudes */
	for(ix=nxf; ix<=nxe; ++ix){
		x = fx+ix*dx;
		dis = (xm>=x)?xm-x:x-xm;
		izt0 = (dis*rxz-fzt)/dzt-1;
		if(izt0<0) izt0 = 0;
		if(izt0>=nzt) izt0 = nzt-1;
		iz0 = (dis*rxz-fz)/dz;
		if(iz0<0) iz0 = 0;
		if(iz0>=nz) iz0 = nz-1;

		ax = (x-fxt)/dxt;
		jx = (int)ax;
		ax = ax-jx;
		if(ax<=0.01) ax = 0.;
		if(ax>=0.99) ax = 1.0;
		ax0 = 1.0-ax;
		if(jx>nxte-1) jx = nxte-1;
		if(jx<nxtf) jx = nxtf;

		ar = (xs>=x)?(xs-x)/dx:(x-xs)/dx;
		jrs = (int)ar;
		if(jrs>=nr-1) jrs = nr-2;
		srs = ar-jrs;
		srs0 = 1.0-srs;
		ar = (xg>=x)?(xg-x)/dx:(x-xg)/dx;
		jrg = (int)ar;
		if(jrg>=nr-1) jrg = nr-2;
		srg = ar-jrg;
		srg0 = 1.0-srg;

		for(iz=izt0; iz<nzt; ++iz){
		    tzt[iz] = ax0*tsum[jx][iz]+ax*tsum[jx+1][iz]
				+srs0*tb[jrs][iz]+srs*tb[jrs+1][iz]
				+srg0*tb[jrg][iz]+srg*tb[jrg+1][iz];

		    amp[iz] = ax0*ampt[jx][iz]+ax*ampt[jx+1][iz];
		    ampi[iz] = ax0*ampti[jx][iz]+ax*ampti[jx+1][iz];
		    zm[iz] = ax0*zmt[jx][iz]+ax*zmt[jx+1][iz];

		}

		nzp = (ax0*zpt[jx]+ax*zpt[jx+1]-fz)/dz+1.5;
		if(nzp<iz0) nzp = iz0;
		if(nzp>nz) nzp = nz;

		/* interpolate along depth if operater aliasing 	*/
		for(iz=iz0; iz<nzp; ++iz) {
			az = z0+iz*rdz;
			jz = (int)az;
			if(jz>=nzt-1) jz = nzt-2;
			sz = az-jz;
			sz0 = 1.0-sz;
			td = sz0*tzt[jz]+sz*tzt[jz+1];
			at = (td-ft)*odt;
			jt = (int)at;
			if(jt > 0 && jt < nt-1){
			    ampd = sz0*ampi[jz]+sz*ampi[jz+1];
			    mz = (int)(0.5+sz0*zm[jz]+sz*zm[jz+1]);
			    res = at-jt;
			    iz1 = iz+mzmax;
 			    temp = (-migi[ix][iz1-mz]+2.0*migi[ix][iz1]
				-migi[ix][iz1+mz])*ampd;
			    trace[jt] += (1.0-res)*temp;
			    trace[jt+1] += res*temp;
			}
		}

		/* interpolate along depth if not operater aliasing 	*/
		for(iz=nzp; iz<nz; ++iz) {
			az = z0+iz*rdz;
			jz = (int)az;
			if(jz>=nzt-1) jz = nzt-2;
			sz = az-jz;
			sz0 = 1.0-sz;
			td = sz0*tzt[jz]+sz*tzt[jz+1];
			at = (td-ft)*odt;
			jt = (int)at;
			if(jt > 0 && jt < nt-1){
			    ampd = sz0*amp[jz]+sz*amp[jz+1];
			    res = at-jt;
			    temp = mig[ix][iz]*ampd;
			    trace[jt] += (1.0-res)*temp;
			    trace[jt+1] += res*temp;
			}
		}
	}



	/* apply half-derivative filter to trace */
	convolve_cwp(nhd,-lhd,hd,nt,0,trace,nt,0,work1);

	/* convolve wavelet with trace */
 	convolve_cwp(w->lw,w->iw,w->wv,nt,0,work1,nt,0,trace);

	/* free workspace */
	free2float(ampt);
	free2float(ampti);
	free1float(tzt);
	free1float(work1);
 	free1float(amp);
 	free1float(ampi);
 	free2float(zmt);
 	free1float(zpt);
 	free1float(zm);
}