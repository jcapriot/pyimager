/* Copyright (c) Colorado School of Mines, 2021.*/
/* All rights reserved.                       */

/* SUDEGHOST: $Revision: 1.7 $ ; $Date: 2021/12/05 05:44:41 $        */


#include "su.h"
#include "segy.h"

/*********************** self documentation **********************/
char *sdoc[] = {
"									",
" SUDEGHOST - applies a DEGHOSTING filter in (t,x) or (tau,p) data	",
"									",
" sudeghost <stdin >stdout [optional parameters]         		",
"									",
" Required parameters:                                         		",
" h=			source or receiver depth			",
"									",
" Optional parameters:							",
" v=1500.0		speed of sound in the top layer			",
" r=0.5			surface reflectivity 0 < r < .8			",
" lambert=1 		Lambert's cosine law obliquity factor		",
" dt= (from header)     time sampling interval (sec)        		",
" verbose=0		=1 for advisory messages, =2 debugging		",
" deghost=1		deghosting filter; =0 ghosting filter for modeling",
" pnoise=1.e-9		white noise parameter				",
"									",
" Notes:								",
" The input data are assumed to be shot gathers with no missing traces.	",
"									",
" If the input data are in the (x,t) domain, and the header fields	",
" f2=0 and d2=0, then the program assumes that the ghost signal is normally",
" incident and the ghost delay time is 2h/v.				",
"									",
" If the input data are in the (tau,p) domain, then it is assumed that  ",
" f2=\'first p value\' and d2=\'increment in p\' and the ghost delay time",
" is assumed to be (2h/v) sqrt{1 - v^2 p^2}, addressing the angular	",
" dependence of the ghost delay. 					",
"									",
" Examples: 								",
" (t,x) domain, streamer depth 10m, surface reflectivity r=0.5		",
"									",
"  sudeghost < input h=10 r=0.5 > output				",
"									",
" (tau,p) domain, streamer depth 10m surface reflectivity r=0.5		",
" min offset=-636 max offset -3237 					",
"									",
" suradon < input choose=0 igopt=3 interoff=-262 offref=-3237 		",
"     pmin=-400 pmax=2000 dp=10 f1=90 f2=125 cdpkey=ep anderson=0  |	",
"     sudeghost h=10 r=.5 deghost=1 |					",
" suradon choose=4 igopt=3 interoff=-262 offref=-3237			", 
"   pmin=-400 pmax=2000 dp=10  f1=90 f2=125 cdpkey=ep anderson=0  > output",
" 									",
" Caveats:								",
" The value of r is the reflectivity of the sea surface, which may be   ",
" both dependent on frequency and on the angle of incidence. The program",
" may be unstable if r > .8 is chosen.					",
" 									",
" Smaller values of dp may be needed to retain high frequencies		",
" 									",
" For lambert=1 a cosine obliquity law is chosen. Frequency dependence  ",
" of reflectivity is not addressed.		 			",
" 									",
" If tr.dt is not set in the header, then dt is mandatory		",
NULL};

/* Credits:
 *      CWP: John Stockwell (November 2021)
 *
 * Technical reference:
 *  Perz, M. J., & Masoomzadeh, H. (2014). Deterministic marine 
 *     deghosting: tutorial and recent advances. GeoConvention. 
 *       Expanded Abstracts.
 * 
 * Trace header fields accessed: ns, dt, d1, f2, d2
 */

/**************** end self doc ***********************************/

/* Prototype of function used internally */
void deGhostingFilter(int lambert, int deghost, float pnoise, int verbose,
			float r, float h, float v, float p,
			 int nfft, float dt, complex *filter);

#define PIBY2   1.57079632679490
#define FRAC0   0.10    /* Ratio of default f1 to Nyquist */
#define FRAC1   0.15    /* Ratio of default f2 to Nyquist */
#define FRAC2   0.45    /* Ratio of default f3 to Nyquist */
#define FRAC3   0.50    /* Ratio of default f4 to Nyquist */
#define LOOKFAC 2       /* Look ahead factor for npfao    */
#define PFA_MAX 720720  /* Largest allowed nfft           */


segy tr;

int
main(int argc, char **argv)
{
        register float *rt=NULL;     /* real trace                           */
        register complex *ct=NULL;   /* complex transformed trace            */
        complex *filter=NULL;    /* filter array                         */

        float dt;               /* sample spacing                       */
        float nyq;              /* nyquist frequency                    */
        int nt;                 /* number of points on input trace      */
        int nfft;               /* number of points for fft trace       */
        int nf;                 /* number of frequencies (incl Nyq)     */
	int verbose;		/* flag to get advisory messages	*/
	cwp_Bool seismic;	/* is this seismic data?		*/

	float r=0.0;		/* surface reflectivity 		*/
	float h=0.0;		/* depth to source or receiver		*/
	float v=0.0;		/* water or top layer velocity		*/
	int ntr=0;		/* trace count 				*/
	float p=0.0;		/* horizontal slowness 			*/
	float fp=0.0;		/* first horizontal slowness 		*/
	float dp=0.0;		/* increment in horizontal slowness 	*/
	int deghost=1;		/* =1 deghost ; =0 add ghosts		*/
	int lambert=1;		/* =1 r cos(theta) ; =0 r		*/

	float pnoise;		/* white noise parameter 		*/
	
        
        /* Initialize */
        initargs(argc, argv);
        requestdoc(1);


        /* Get info from first trace */ 
	if (!getparint("verbose", &verbose))	verbose=0;
        if (!gettr(&tr))  err("can't get first trace");
	seismic = ISSEISMIC(tr.trid);
	if (seismic) {
		if (verbose)	warn("input is seismic data, trid=%d",tr.trid);
		dt = ((double) tr.dt)/1000000.0;
	}
	else {
		if (verbose) warn("input is not seismic data, trid=%d",tr.trid);

		dt = tr.d1;

        }

	/* error trapping so that the user can have a default value of dt */
	if (!(dt || getparfloat("dt", &dt))) {
		dt = .004;
		if (verbose) {
			warn("neither dt nor d1 are set, nor is dt getparred!");
			warn("assuming .004 sec sampling!");
		}
	}

	/* read parameters from the headers */
        nt = tr.ns;
        nyq = 0.5/dt;
	fp=tr.f2;
	dp=tr.d2;

	/* required parameter */
	MUSTGETPARFLOAT("h", &h);

	/* optional parameters */
	if (!getparfloat("r", &r))			r=0.5;
	if (!getparfloat("v", &v))			v=1500.0;
	if (!getparint("deghost",&deghost))		deghost=1;
	if (!getparint("lambert",&lambert))		lambert=1;

	if (!getparfloat("pnoise", &pnoise)) pnoise=1.0e-9;

        /* Set up FFT parameters */
        nfft = npfaro(nt, LOOKFAC * nt);
        if (nfft >= SU_NFLTS || nfft >= PFA_MAX)
                err("Padded nt=%d -- too big", nfft);

        nf = nfft/2 + 1;


        /* Allocate fft arrays */
        rt   = ealloc1float(nfft);
        ct   = ealloc1complex(nf);
        filter = ealloc1complex(nf);


        /* Main loop over traces */
        do {
                register int i;

		++ntr;

		/* calculate p horizontal slowness */
		p = ntr*dp + fp;

		/* Build the deghosting filter */
		deGhostingFilter(lambert,deghost,pnoise,verbose,
					r,h,v,p,nfft,dt,filter);

                /* Load trace into rt (zero-padded) */
                memcpy((void *) rt, (const void *) tr.data, nt*FSIZE);
                memset((void *) (rt + nt), 0 , (nfft-nt)*FSIZE);

                /* FFT, filter, inverse FFT */
                pfarc(1, nfft, rt, ct);
                for (i = 0; i < nf; ++i)  ct[i] = cmul(ct[i], filter[i]);
                pfacr(-1, nfft, ct, rt);

                /* Load traces back in, recall filter had nfft factor */
                for (i = 0; i < nt; ++i)  tr.data[i] = rt[i];

		if (verbose) warn("r = %f nyq = %f tr.f2 = %f  tr.d2 = %f",r,nyq,tr.f2,tr.d2);
                puttr(&tr);
        } while (gettr(&tr));

        return(CWP_Exit());
}


void deGhostingFilter(int lambert, int deghost,float pnoise,int verbose,
			float r, float h, float v, float p,
			int nfft, float dt, complex *filter)
/*************************************************************************
DEGHOSTING filter applied in the (t,x) or (tau,p) domain
**************************************************************************
Input:
lambert		=1 r cos(theta) ; =0  r
deghost		=1 deghost ; =0 add ghosts for modeling
pnoise		white noise parameter
r		absolute value of surface reflectivity
v		water velocity or velocity of top layer
h		depth of source or receiver
p               horizontal slowness
nfft		number size of the fft
dt		time sampling interval in seconds

Output:
filter		array[nfft] filter values
**************************************************************************
Theory:

For data in the (x,t) domain p=0 and normal incidence is assumed.
In general, the ghosting operator is both frequency and angularly dependent 
G(\omega,p) = 1 - r(\omega,p)exp{i omega (2h/v)sqrt(1 - v^2 p^2) } 

implying that the deghosting operator is the inverse

G^(-1) (\omega,p) = 1/{1 - r(\omega,p)exp(i omega (2h/v)sqrt{1 - v^2 p^2 )}

If lambert=1 then Lambert's cosine amplitude obliquity law is assumed. For
lambert=0, then  r is a constant 0 < r < 1.0

The filter may be further simplified by writing the complex exponentials as
exp(i phi) = cos(phi) + i sin(phi), and multiplying the numerator and 
denominator by its complex conjugate.

G^(-1)(\omega,p) is complex valued

Re[G^(-1)] = 1- r cos(omega (2h/v)sqrt{1 - v^2 p^2} )/modulus_squared

Im[G^(-1)] = r sin(omega (2h)/v) sqrt{1 - v^2 p^2} )/modulus_squared

where
modulus_squared = | 1 - r cos(omega(2h/v) sqrt{1 - v^2 p^2} |^2 
                   +| r sin(omega(2h/v) sqrt{1 - v^2 p^2} |^2 
**************************************************************************
Technical reference:
  Perz, M. J., & Masoomzadeh, H. (2014). Deterministic marine 
     deghosting: tutorial and recent advances. GeoConvention. 
       Expanded Abstracts.
**************************************************************************
Notes: Filter is to be applied in the frequency domain
**************************************************************************
Author:  CWP: John Stockwell   2021
*************************************************************************/
#define PIBY2   1.57079632679490
#define TWOPI           2.0 * PI
{
        int ifreq;		/* loop counting variables              */
	float real;		/* real part				*/
	float imag;		/* imaginary part			*/
	float denom;		/* denominator				*/
	float twowaytt;		/* two way traveltime			*/
	float onfft;		/* 1.0/nfft 				*/
	float omega;		/* angular frequency 			*/
	float nf;		/* number of frequencies 		*/
	float df;		/* increment in frequency 		*/
	float amp=0.0;		/* amplitude				*/

	
	/* set sizes and increments */
        nf = nfft/2 + 1;
        onfft = 1.0 / nfft;
	df = onfft / dt;
	
	/* Build filter */
	for (ifreq = 0; ifreq < nf ; ++ifreq) {
		float vvpp=v*v*p*p;
		float f = ifreq * df;

		/* the  pv > 1.0 is forbidden */
		if (vvpp>1) vvpp=1.0;

		/* scale the amplitude by cos(theta) for lambert=1 */
		if (lambert==1) {
			amp = r*sqrt(1 - vvpp); 
			if (amp < pnoise) amp=pnoise;
		} else if (lambert==0) {
			amp = r;
		}

		/* compute quantities that constitute the filter */
		omega = TWOPI * f;
		twowaytt = 2*h*sqrt(1 - vvpp)/v;
		real = 1 - amp*cos(omega*twowaytt);
		imag = amp*sin(omega*twowaytt);
		denom = real*real + imag*imag;

		if (verbose==2) /* debugging */
			warn("amp = %f lambert = %d deghost = %d f = %f df = %f omega = %f twowaytt = %f dt = %f ",amp,lambert,deghost,f,df,omega,twowaytt,dt);

		if (deghost==1){ /* deghosting filter */
		
			filter[ifreq].r = onfft * real/denom; 
			filter[ifreq].i = onfft * imag/denom;

		} else if (deghost==0) { /* ghosting filter for modeling */
	
			filter[ifreq].r = onfft * real; 
			filter[ifreq].i = -onfft * imag;
		}
	}

}
