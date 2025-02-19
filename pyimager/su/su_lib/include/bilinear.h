
void binterpfind(int kigi, int *mgi, int mgi_tot, int mgiextr,
                 int kigc, int *mgc, int mgc_tot, int mgcextr,
                 int *mgix, int *mgcx, double *wi, double *wc) ;

void binterpapply(double *lwitims, double *hiitims, int mgi_tot, double wi,  
                  double *lwctims, double *hictims, int mgc_tot, double wc,
                  int lwinto, double *valsout) ;

void binterpvalue(double offset, int mgtextr,
                  double *lwioffs, double *lwitims, int lwinto,
                  double *hiioffs, double *hiitims, int hiinto,
                  int mgi_tot, double wi,
                  double *lwcoffs, double *lwctims, int lwcnto,
                  double *hicoffs, double *hictims, int hicnto,
                  int mgc_tot, double wc,
                  double *timeout) ;
