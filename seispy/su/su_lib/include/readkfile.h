/* Copyright (c) Colorado School of Mines, 2021.*/
/* All rights reserved.                       */

/* readkfile.h: $Revision: 1.1 $ ; $Date: 2021/10/03 23:15:33 $        */

#ifndef READKFILE_H
#define READKFILE_H


void readkfile(FILE *fpR, cwp_String *names, cwp_String *forms, double *dfield,
               int *numcases, int *errwarn) ;

void writekfile(FILE *fpW, cwp_String *names, cwp_String *forms, double *dfield,
               int numcasesout, int *errwarn) ;

#endif /* end READKFILE_H */
