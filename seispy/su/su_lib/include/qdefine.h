
#ifndef QDEFINE_H
#define QDEFINE_H

struct  QInfo { 
     int *kinf;
     int nto;
     double *dlots;
};

void getviacommand(cwp_String **pnameA, int *numpnameA, int *iztupleA, int numdind,
                   int *ktuple, int *ifixd, struct QInfo **RecInfoA, int *ncdpA,
                   double **pindepaA, cwp_String **ndimsA, int *errwarn) ;

void getqinfo(FILE *fpR, int *num_names, int *num_ndims, 
              int iqflag, int *ibsize, int *ivsize, int *irtotal, int *errwarn) ;

void readqhead(FILE *fpR, char *textraw, char *textbeg, int maxtext, 
               cwp_String *names, cwp_String *ndims,
               int *num_names, int *num_ndims, int *errwarn) ;

void getviaqfile(FILE *fpP, cwp_String **pnameA, int *numpnameA, int *iztupleA, int numdind,   
                 int *ktuple, int *ifixd, struct QInfo **RecInfoA, int *ncdpA, 
                 double **pindepaA,  cwp_String **ndimsA, int *errwarn) ;

void qelementout(int iztuple,int ktuple,int numdind,int *kindx,int *ksize) ;

void qsplit(struct QInfo *RecInfo, int ncdp,
            int **ngi, int *ngi_tot, int **ngc, int *ngc_tot, int *errwarn) ;

void getqvalscsv(char *textraw, char *textbeg, int maxtext, char rdel,
                 int *nspot, int numcases, double *dfield, int *izlast, int *notblank, int *errwarn) ;

void tparseq(char *tbuf, char d, char **fields, int *numfields) ; 

#endif
