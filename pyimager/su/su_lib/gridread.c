#include "su.h"
#include "readkfile.h"
#include "gridread.h"

void gridread(FILE *fpR, double *gvals, int *errwarn2) 
{ 

/* Arguments:                                                     */    
/*  fpR          the open FILE * for K-file containing the grid.  */    
/*               if NULL, all grid parameters must be specified   */    
/*               on the command line. If parameters exist in both,*/    
/*               the command line values override K-file values.  */    
/*                                                                */    
/*  gvals        pointer to double array of at least 20 elements. */    
/*               Will contain processed grid values on return.    */    
/*                                                                */    
/*  *errwarn2    if 0 on input, do not print anything on error    */    
/*               or unusual thing. Will still contain error/warn  */    
/*               codes on return. Any returned value >0 means the */    
/*               grid is not usable.                              */    
/*                                                                */    
/* Note: The 999 arrays are much bigger than will be used.        */    
/* But clumsy to dynamically allocate since they are              */    
/* read-in from C_SU_ records in the input k-file.                */    

  cwp_String names[999];   
  cwp_String forms[999];   
  double dfield[999];
  cwp_String gnams[999];   

  int i=0;
  int j=0;
  int n=0;
  int m=0;
/* if *errwarn2 input 0, do not print (still return error code)   */
  int iwarn = *errwarn2; 
  *errwarn2 = 0;

  int numcases = 0;
  int errwarn = 0;

  if(fpR!=NULL) readkfile(fpR,names,forms,dfield,&numcases,&errwarn);
     
  if(iwarn != 0) {
    if(errwarn==1) warn("K-file read error: more than one C_SU_NAMES parameter record.");
    else if(errwarn==2) warn("K-file read error: no C_SU_NAMES parameter record.");
    else if(errwarn==3) warn("K-file read error: more than one C_SU_FORMS parameter record.");
    else if(errwarn==4) warn("K-file read error: no C_SU_FORMS parameter record.");
    else if(errwarn==5) warn("K-file read error: more than one C_SU_SETID record.");
    else if(errwarn==6) warn("K-file read error: no C_SU_SETID record.");
    else if(errwarn==7) warn("K-file read error: different number of values on C_SU_NAMES and C_SU_FORMS.");
    else if(errwarn==8) warn("K-file read error: unable to allocate memory.");
    else if(errwarn==9) warn("K-file read error: name exists at least twice in C_SU_NAMES list.");
    else if(errwarn==10) warn("K-file read error: at least 1 field-unreadable as a number.");
    else if(errwarn==11) warn("K-file read error: at least 1 field containing 2 numbers.");
    else if(errwarn==12) warn("K-file read error: not-enough-commas to get all values for C_SU_NAMES list.");
    else if(errwarn>0) warn("K-file read error: returned with some unrecognized error code.");
    else if(errwarn==-1) warn("K-file read warning: at least 1 all-blank field, assumed zero for all.");
  } 
 
  *errwarn2 = errwarn;
  if(errwarn>0) return; 

  for (n=0; n<numcases; n++) {
    for (m=0; m<strlen(names[n]); m++) {
      names[n][m] = tolower(names[n][m]);
    }
  }    

  int numgnams = 18;

  for(i=1; i<numgnams; i++) {
    gvals[i] = -1.1e308;
    gnams[i] = ealloc1(7,1); 
  }

  strcpy(gnams[1],"grid_lf");
  strcpy(gnams[2],"grid_xa");
  strcpy(gnams[3],"grid_ya");
  strcpy(gnams[4],"grid_xb");
  strcpy(gnams[5],"grid_yb");
  strcpy(gnams[6],"grid_xc");
  strcpy(gnams[7],"grid_yc");
  strcpy(gnams[8],"grid_xd");
  strcpy(gnams[9],"grid_yd");
  strcpy(gnams[10],"grid_wb");
  strcpy(gnams[11],"grid_wc");
  strcpy(gnams[12],"grid_nb");
  strcpy(gnams[13],"grid_nc");
  strcpy(gnams[14],"grid_fp");
  strcpy(gnams[15],"grid_lp");
  strcpy(gnams[16],"grid_sb");
  strcpy(gnams[17],"grid_cb");

  for(i=2; i<12; i++) {    /* read-in corners A,B,C and cell widths */ 
    if(i==8 || i==9) continue; /* do not read-in corner D  */
    if(!getpardouble(gnams[i],gvals+i)) { 
      for(j=0; j<numcases; j++) { 
        if(strcmp(names[j],gnams[i]) == 0) gvals[i] = dfield[j]; 
      }
      if(gvals[i] < -1.e308) {
        gvals[i] = i+100;
        if(iwarn != 0) warn("grid parameter error: %s not found.",gnams[i]); 
        *errwarn2 = 20;
        return; 
      }
    }
  } /* end of  for(i=2; i<12; i++) { */

  return;
}
/* -----------------------------------------------------------    */
void gridcommand(int *enough) 
{ 

/* A primary purpose of this routine is to know when it is        */    
/* NOT necessary to have/open the k-file before gridread.         */    
/*                                                                */    
/* Output Argument:                                               */    
/*  enough =  1 means all of the required grid parameters         */    
/*              exist on the command line.                        */    
/*         =  0 means none of the required grid parameters        */    
/*              exist on the command line.                        */    
/*         = -1 means some of the required grid parameters        */    
/*              exist on the command line.                        */    
/*                                                                */    
/* The required grid parameters are: grid_xa,grid_ya,             */    
/* grid_xb,grid_yb, grid_xc,grid_yc, grid_wb, grid_wc.            */    
/* (The x,ys for corners A,B,C and cell widths A-->B and A-->C).  */    

  cwp_String gnams[18];   
  int numgnams = 18;

  int i=0;

  for(i=1; i<numgnams; i++) gnams[i] = ealloc1(7,1); 

  strcpy(gnams[1],"grid_lf");
  strcpy(gnams[2],"grid_xa");
  strcpy(gnams[3],"grid_ya");
  strcpy(gnams[4],"grid_xb");
  strcpy(gnams[5],"grid_yb");
  strcpy(gnams[6],"grid_xc");
  strcpy(gnams[7],"grid_yc");
  strcpy(gnams[8],"grid_xd");
  strcpy(gnams[9],"grid_yd");
  strcpy(gnams[10],"grid_wb");
  strcpy(gnams[11],"grid_wc");
  strcpy(gnams[12],"grid_nb");
  strcpy(gnams[13],"grid_nc");
  strcpy(gnams[14],"grid_fp");
  strcpy(gnams[15],"grid_lp");
  strcpy(gnams[16],"grid_sb");
  strcpy(gnams[17],"grid_cb");

/* kept the above array the same as gridset, just to ease understanding.  */

  double gval;
  int nfound = 0;
  for(i=2; i<12; i++) {    /* need corners A,B,C and cell widths      */ 
    if(i==8 || i==9) continue; /* do not need corner D (or anything else) */
    if(getpardouble(gnams[i],&gval)) nfound++;
  } 

  *enough = 0;
  if(nfound==8) *enough = 1; 
  else if(nfound>0) *enough = -1;

  return;
}

/* -----------------------------------------------------------    */

