
/* #include <stdio.h> */
/* #include <string.h> */

#include "su.h"
#include "readkfile.h"
/* #include <segy.h> */

void getkvalscsv(char *textraw, char *textbeg, int maxtext, char rdel, 
                 double *dfield, int *nspot, int numcases,   
                 int ncount, int *comerr,int *morerr,int *numerr,int *nblank) ;

void tparse(char *tbuf, char d, char **fields, int *numfields) ; 


/* Input Argument:                                                                  */
/* fpR         = open FILE*                                                         */
/*                                                                                  */
/* Output Arguments:                                                                */
/* names       = names   read from record after C_SU_NAMES record                   */
/* forms       = formats read from record after C_SU_FORMS record                   */
/* dfield      = values read from K-record                                          */
/* numcasesout = number of names,forms,dfield                                       */
/* errwarn    >0 means some kind of error                                           */
/*            =1 more than one C_SU_NAMES parameter record.                         */
/*            =2 no C_SU_NAMES parameter record.                                    */ 
/*            =3 more than one C_SU_FORMS parameter record.                         */
/*            =4 no C_SU_FORMS parameter record.                                    */  
/*            =5 more than one C_SU_SETID record.                                   */   
/*            =6 no C_SU_SETID record.                                              */   
/*            =7 different number of values on C_SU_NAMES and C_SU_FORMS.           */   
/*            =8 unable to allocate memory.                                         */
/*            =9 name exists at least twice in C_SU_NAMES list.                     */
/*            =10 at least 1 field-unreadable as a number.                          */
/*            =11 at least 1 field containing 2 numbers.                            */  
/*            =12 not-enough-commas to get all dfield for C_SU_NAMES numcasesout.   */
/*            =-1 at least 1 all-blank field, assumed zero for all.                 */

void readkfile(FILE *fpR, cwp_String *names, cwp_String *forms, double *dfield, 
               int *numcasesout, int *errwarn) 
{

  *errwarn = 0;

  int i;
  int n;
  int m;
  int numcases = 0;

  int maxtext = 10001;
  char textraw[10001]; /* fgets puts a \0 after contents */
  char textbeg[10001]; /* so this size wastes memory but not time */
  char textraw2[10001];
  char textfront[10];  

  int *nspot = NULL;

  char rdel = ',';

  cwp_String Rid  =NULL;  /* rejection id for records             */
  Rid = ealloc1(1,1);
  strcpy(Rid,"K");
        
  int num_names = 0;
  int num_forms = 0;

  int num_c_su_names = 0;
  int num_c_su_forms = 0;
  int num_c_su_setid = 0;
  int read_names = 0;
  int read_forms = 0;

  while (fgets(textraw, maxtext, fpR) != NULL) { /* read a line */

/* Stop this looping? Sometimes, it really will just loop through to last record. */
                   
    if(read_names==-1 && read_forms==-1) break;

/* Remove all blanks and tabs because tparse is not designed to handle them.      */

    int tsize = 0;
    for(n=0; n<maxtext; n++) { /*   linux \n            windows \r */
      if(textraw[n] == '\0' || textraw[n] == '\n' || textraw[n] == '\r') break;
      if(textraw[n] != ' ' && textraw[n] != '\t') {
        textbeg[tsize] = textraw[n];
        tsize++;
      }
    }

    for(n=0; n<sizeof(textbeg); n++) textbeg[n] = tolower(textbeg[n]);
    if(strncmp(textbeg,"c_su_setid",10) == 0) num_c_su_setid++;
    if(strncmp(textbeg,"c_su_names",10) == 0) num_c_su_names++;
    if(strncmp(textbeg,"c_su_forms",10) == 0) num_c_su_forms++;

    if(read_names>0) {
      textbeg[tsize] = '\0'; 
      tparse(textbeg, ',', names, &num_names) ; 
      read_names = -1;
    }

    if(strncmp(textbeg,"c_su_names",10) == 0) read_names = 1;

    if(read_forms>0) {
      textbeg[tsize] = '\0';  
      tparse(textbeg, ',', forms, &num_forms) ; 
      read_forms = -1;
    }

    if(strncmp(textbeg,"c_su_forms",10) == 0) read_forms = 1;

  } /* end of while (fgets(textraw,..... */
  
  fseek(fpR, 0L, SEEK_SET); /* reposition file to beginning record */ 

  if(num_c_su_names>1) {
    *errwarn = 1;
    return;
  }
  else if(num_c_su_names==0) {
    *errwarn = 2;
    return;
  }

  if(num_c_su_forms>1) {
    *errwarn = 3;
    return;
  }
  else if(num_c_su_forms==0) {
    *errwarn = 4;
    return;
  }

  if(num_c_su_setid>1) {
    *errwarn = 5;
    return;
  }
  else if(num_c_su_setid==0) {
    *errwarn = 6;
    return;
  }
  
  if(num_forms != num_names) {
    *errwarn = 7;
    return;
  }

  nspot = ealloc1int(num_names);
  if(nspot == NULL) {    
    *errwarn = 8;
    return;
  }

  if(strncmp(names[0],"c_su_id",7) == 0) names[0] = "null";

  for (n=0; n<num_names; n++) {
    if(strncmp(names[n],"null",4) != 0) {
      for (m=n+1; m<num_names; m++) {
        if(strcmp(names[n],names[m]) == 0) {  
          *errwarn = 9;
          return;
        }
      }
    }
  }

/* ----------------------------------------------------- */
/* ----------------------------------------------------- */

  for(i=0; i<num_names;i++) { 
    if(strncmp(names[i],"null",4) != 0) { /* actually removes c_su_id also */
      names[numcases] = names[i];
      forms[numcases] = forms[i];
      nspot[numcases] = i;
      numcases++; 
    }
  }

  memset(textraw,'\0',10001);
  memset(textraw2,'\0',10001);
  memset(textbeg,'\0',10001);

  int ncount = 0;
  int comerr = 0;
  int morerr = 0;
  int numerr = 0;
  int nblank = 0;
  int nextrow = 0;

  while (fgets(textraw, maxtext, fpR) != NULL) { /*read a line*/
    ncount++;
    for(n=0; n<10; n++) textfront[n] = tolower(textraw[n]);
    if(strncmp(textfront,"c_su",4) == 0 || nextrow==1) {
      nextrow = 0; 
      if(strncmp(textfront,"c_su_names",10) == 0 || 
         strncmp(textfront,"c_su_forms",10) == 0) nextrow = 1;
    }
    else {
      if(strncmp(textraw,Rid,1) == 0) { /* Rid compare is case-sensitive */

        getkvalscsv(textraw, textbeg, maxtext, rdel, 
                    dfield, nspot, numcases,
                    ncount, &comerr,&morerr,&numerr,&nblank);

        break;

      }  
    }
  }

  if(numerr>0) {
    *errwarn = 10;
    return;
  }
  if(morerr>0) {
    *errwarn = 11;
    return;
  }
  if(comerr>0) {
    *errwarn = 12;
    return;
  }

  if(nblank>0) *errwarn = -1;

  *numcasesout = numcases;

}    

void getkvalscsv(char *textraw, char *textbeg, int maxtext, char rdel, 
                 double *dfield, int *nspot, int numcases,   
                 int ncount, int *comerr,int *morerr,int *numerr,int *nblank) 
{
  int n;
  int m;
  int nbeg = -1;
  int nfield = 0;
  int ineed  = 0;
  int igot;
  double dval;
  for(n=0; n<maxtext; n++) {                         /* linux \n            windows \r */
    if(textraw[n] == rdel || textraw[n] == '\0' || textraw[n] == '\n' || textraw[n] == '\r') {
      if(nfield == nspot[ineed]) {
        dval = 1.1e308; 
        int nb = -1;
        if(n-nbeg-1 > 0) {
          strncpy(textbeg,textraw+nbeg+1,n-nbeg-1);
          textbeg[n-nbeg-1] = '\0'; /* so sscanf knows where to stop */
          int ib = -1;
          for (m=0; m<n-nbeg-1; m++) {
            if(textbeg[m] != ' ') {
              nb = m;
              if(ib>-1) {
                *morerr = *morerr + 1;
                nb=-1;
                break;
              }
            }
            if(textbeg[m] == ' ' && nb>-1) ib = m;
          }  

          if(nb>-1) {
            igot = sscanf(textbeg,"%lf",&dval);  
            if(igot<1) *numerr = *numerr + 1;
          }
        } /* end of  if(n-nbeg-1 > 0) { */
        if(nb<0) {
          *nblank = *nblank + 1;
           dval = 0.; 
        }
        dfield[ineed] = dval;
        ineed++;
        if(ineed>=numcases) break; 
      }
      if(textraw[n] == '\0' || textraw[n] == '\n' || textraw[n] == '\r') {
        if(ineed<numcases) *comerr = *comerr + 1;
      }
      nbeg = n;
      nfield++;
    }
  } 
  return;
}

/* --------------------------- */
/* expects a string with no blanks and no tabs \t   */
void tparse(char *tbuf, char d, char **fields, int *numfields) 
{ 
  int n=0;
  int nbeg = -1;
  *numfields = 0;
  for(n=0; ; n++) {
    if(tbuf[n] == d || tbuf[n] == '\0') {
      if(n-nbeg-1 > 0) {
        fields[*numfields] = ealloc1(n-nbeg-1,1);
        strncpy(fields[*numfields],tbuf+nbeg+1,n-nbeg-1);
      }
      else {
        fields[*numfields] = ealloc1(4,1);
        fields[*numfields][0] = 'n';
        fields[*numfields][1] = 'u';
        fields[*numfields][2] = 'l';
        fields[*numfields][3] = 'l';
/*      strncpy(fields[*numfields],"null",4);  makes compilor unhappy */
      }
      nbeg = n;
      *numfields = *numfields + 1;
    }
    if(tbuf[n] == '\0') break;
  }
 
}

/* Input Arguments:                                                                 */
/* fpW         = open FILE*                                                         */
/* names       = names   to output to record after C_SU_NAMES record                */
/* forms       = formats to output to record after C_SU_FORMS record                */
/* dfield      = values to output to K-record                                       */
/* numcasesout = number of names,forms,dfield                                       */
/* errwarn    >0 means some kind of error                                           */

void writekfile(FILE *fpW, cwp_String *names, cwp_String *forms, double *dfield, 
                int numcasesout, int *errwarn) 
{

  int i=0;
  int ineed=0;
  *errwarn = 0;

  char textraw[10001]; /* fgets puts a \0 after contents */
  char textbeg[10001]; /* so this size wastes memory but not time */
  char textraw2[10001];
        
  cwp_String Rid  =NULL;  /* rejection id for records             */
  Rid = ealloc1(1,1);
  strcpy(Rid,"K");
        
  memset(textraw,'\0',10001);
  memset(textraw2,'\0',10001);
  memset(textbeg,'\0',10001);

  int mspot;
  int mleng;

  strcpy(textraw,"C_SU_SETID,");
  strcpy(textraw+11,Rid);
  textraw[12] = '\n';
  textraw[13] = '\0';
  fputs(textraw,fpW);

/* write the forms record                        */ 

  strcpy(textraw,"C_SU_FORMS");
  textraw[10] = ' ';
  textraw[10] = '\n';
  textraw[11] = '\0';
  fputs(textraw,fpW);

  mspot = 8;
  strcpy(textraw,"C_SU_ID,");

  for(i=0; i<numcasesout; i++) { 
    mleng = strlen(forms[i]);
    strncpy(textraw+mspot,forms[i],mleng);
    mspot += mleng;
    textraw[mspot] = ',';
    mspot++;
  } /* end of  for(i=0; i<numcases; i++) { */ 
  textraw[mspot-1] = '\n';
  textraw[mspot  ] = '\0';
  fputs(textraw,fpW);

/* write the names record                        */ 

  strcpy(textraw,"C_SU_NAMES");
  textraw[10] = '\n';
  textraw[11] = '\0';
  fputs(textraw,fpW);

  mspot = 8;
  strcpy(textraw,"C_SU_ID,");

  for(i=0; i<numcasesout; i++) { 
    mleng = strlen(names[i]);
    strncpy(textraw+mspot,names[i],mleng);
    mspot += mleng;
    textraw[mspot] = ',';
    mspot++;
  } /* end of  for(i=0; i<numcases; i++) { */ 
  textraw[mspot-1] = '\n';
  textraw[mspot  ] = '\0';
  fputs(textraw,fpW);

  strncpy(textraw2,Rid,1);
  int mhere = 1;
  for(ineed=0; ineed<numcasesout; ineed++) {  
    if(dfield[ineed]<1.e308) {
      sprintf(textbeg,forms[ineed],dfield[ineed]);
      int mfill = strlen(textbeg);
      textraw2[mhere] = ','; /* always comma for output */
      strncpy(textraw2+mhere+1,textbeg,mfill);
      mhere += mfill+1;
    }
    else {
      textraw2[mhere] = ',';
      textraw2[mhere+1] = '*';
      mhere += 2;
    }
  }
  textraw2[mhere]   = '\n';
  textraw2[mhere+1] = '\0';
  fputs(textraw2,fpW);

}    
