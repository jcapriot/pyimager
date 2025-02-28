/*  #include "segy.h" */

#include "su.h"
#include "segy.h"
#include "headcase.h"

/* Set Case Number from Key Names.                                                    */
/*                                                                                    */
/* Input arguments:                                                                   */
/*  cbuf = the name of a trace header key (tracl,fldr,cdp,sx, and so on).             */
/*         The following special names are recognized and used within some programs.  */
/*         Any name starting with null returns case 0.                                */
/*         Any name starting with numbM (where M is an integer) returns case 1000+M.  */
/*                                                                                    */
/* Return: Case number for use in fromhead and tohead.                                */
/*         If -1, name was not recognized.                                            */
/*         Current range of trace header key case numbers is 1 to 80 (Feb 2022).      */
/*         But do not presume that case numbers will retain their mapping to any      */
/*         particular key name. And do not presume no more will be added.             */

int GetCase(char* cbuf) {
   
       int ncase = -1;
   
       if(strncmp(cbuf,"null",4) == 0) ncase = 0;  /* any name starting with null */
       else if(strcmp(cbuf,"tracl") == 0) ncase = 1;
       else if(strcmp(cbuf,"tracr") == 0) ncase = 2;
       else if(strcmp(cbuf,"fldr" ) == 0) ncase = 3;
       else if(strcmp(cbuf,"tracf") == 0) ncase = 4;
       else if(strcmp(cbuf,"ep"   ) == 0) ncase = 5;
       else if(strcmp(cbuf,"cdp") == 0) ncase = 6;
       else if(strcmp(cbuf,"cdpt") == 0) ncase = 7;
       else if(strcmp(cbuf,"trid") == 0) ncase = 8;
       else if(strcmp(cbuf,"nvs") == 0) ncase = 9;
       else if(strcmp(cbuf,"nhs") == 0) ncase = 10;
       else if(strcmp(cbuf,"duse") == 0) ncase = 11;
       else if(strcmp(cbuf,"offset") == 0) ncase = 12;
       else if(strcmp(cbuf,"gelev") == 0) ncase = 13;
       else if(strcmp(cbuf,"selev") == 0) ncase = 14;
       else if(strcmp(cbuf,"sdepth") == 0) ncase = 15;
       else if(strcmp(cbuf,"gdel") == 0) ncase = 16;
       else if(strcmp(cbuf,"sdel") == 0) ncase = 17;
       else if(strcmp(cbuf,"swdep") == 0) ncase = 18;
       else if(strcmp(cbuf,"gwdep") == 0) ncase = 19;
       else if(strcmp(cbuf,"scalel") == 0) ncase = 20;
       else if(strcmp(cbuf,"scalco") == 0) ncase = 21;
       else if(strcmp(cbuf,"sx") == 0) ncase = 22;
       else if(strcmp(cbuf,"sy") == 0) ncase = 23;
       else if(strcmp(cbuf,"gx") == 0) ncase = 24;
       else if(strcmp(cbuf,"gy") == 0) ncase = 25;
       else if(strcmp(cbuf,"counit") == 0) ncase = 26;
       else if(strcmp(cbuf,"wevel") == 0) ncase = 27;
       else if(strcmp(cbuf,"swevel") == 0) ncase = 28;
       else if(strcmp(cbuf,"sut") == 0) ncase = 29;
       else if(strcmp(cbuf,"gut") == 0) ncase = 30;
       else if(strcmp(cbuf,"sstat") == 0) ncase = 31;
       else if(strcmp(cbuf,"gstat") == 0) ncase = 32;
       else if(strcmp(cbuf,"tstat") == 0) ncase = 33;
       else if(strcmp(cbuf,"laga") == 0) ncase = 34;
       else if(strcmp(cbuf,"lagb") == 0) ncase = 35;
       else if(strcmp(cbuf,"delrt") == 0) ncase = 36;
       else if(strcmp(cbuf,"muts") == 0) ncase = 37;
       else if(strcmp(cbuf,"mute") == 0) ncase = 38;
       else if(strcmp(cbuf,"ns") == 0) ncase = 39;
       else if(strcmp(cbuf,"dt") == 0) ncase = 40;
       else if(strcmp(cbuf,"gain") == 0) ncase = 41;
       else if(strcmp(cbuf,"igc") == 0) ncase = 42;
       else if(strcmp(cbuf,"igi") == 0) ncase = 43;
       else if(strcmp(cbuf,"corr") == 0) ncase = 44;
       else if(strcmp(cbuf,"sfs") == 0) ncase = 45;
       else if(strcmp(cbuf,"sfe") == 0) ncase = 46;
       else if(strcmp(cbuf,"slen") == 0) ncase = 47;
       else if(strcmp(cbuf,"styp") == 0) ncase = 48;
       else if(strcmp(cbuf,"stas") == 0) ncase = 49;
       else if(strcmp(cbuf,"stae") == 0) ncase = 50;
       else if(strcmp(cbuf,"tatyp") == 0) ncase = 51;
       else if(strcmp(cbuf,"afilf") == 0) ncase = 52;
       else if(strcmp(cbuf,"afils") == 0) ncase = 53;
       else if(strcmp(cbuf,"nofilf") == 0) ncase =54;
       else if(strcmp(cbuf,"nofils") == 0) ncase = 55;
       else if(strcmp(cbuf,"lcf") == 0) ncase = 56;
       else if(strcmp(cbuf,"hcf") == 0) ncase = 57;
       else if(strcmp(cbuf,"lcs") == 0) ncase = 58;
       else if(strcmp(cbuf,"hcs") == 0) ncase = 59;
       else if(strcmp(cbuf,"year") == 0) ncase = 60;
       else if(strcmp(cbuf,"day") == 0) ncase = 61;
       else if(strcmp(cbuf,"hour") == 0) ncase = 62;
       else if(strcmp(cbuf,"minute") == 0) ncase = 63;
       else if(strcmp(cbuf,"sec") == 0) ncase = 64;
       else if(strcmp(cbuf,"timbas") == 0) ncase = 65;
       else if(strcmp(cbuf,"trwf") == 0) ncase = 66;
       else if(strcmp(cbuf,"grnors") == 0) ncase = 67;
       else if(strcmp(cbuf,"grnofr") == 0) ncase = 68;
       else if(strcmp(cbuf,"grnlof") == 0) ncase = 69;
       else if(strcmp(cbuf,"gaps") == 0) ncase = 70;
       else if(strcmp(cbuf,"otrav") == 0) ncase = 71;
       else if(strcmp(cbuf,"d1") == 0) ncase = 72;
       else if(strcmp(cbuf,"f1") == 0) ncase = 73;
       else if(strcmp(cbuf,"d2") == 0) ncase = 74;
       else if(strcmp(cbuf,"f2") == 0) ncase = 75;
       else if(strcmp(cbuf,"ungpow") == 0) ncase = 76;
       else if(strcmp(cbuf,"unscale") == 0) ncase = 77;
       else if(strcmp(cbuf,"ntr") == 0) ncase = 78;
       else if(strcmp(cbuf,"mark") == 0) ncase = 79;
       else if(strcmp(cbuf,"shortpad") == 0) ncase = 80;
       else if(strncmp(cbuf,"numb",4) == 0) {
         ncase = 1000 + atoi(cbuf+4);
       }
  
   return ncase;

}
/* ---------------------------------------------------------------------------------- */
/* Get Value from trace header using case number from GetCase.                        */
/*                                                                                    */
/* Input arguments:                                                                   */
/*    tr = a trace.                                                                   */
/*    k  = the case number of the key name passed to GetCase.                         */
/*                                                                                    */
/* Return: value from header of this case number (key).                               */
/*         If case number is not >0 and <81, nothing is returned.                     */
/*                                                                                    */
/* You get the raw header value (scalel and scalco flags are not considered).         */
/*                                                                                    */
double fromhead(segy tr, int k) {

       double dval;

       switch (k) {
   
         case -1: 
/*       null, name not found? */
         break;
         case 0:  
/*       null   do not read from header */ 
         break;
         case 1:
           dval = tr.tracl;
         break;
         case 2:
           dval = tr.tracr;
         break;
         case 3:
           dval = tr.fldr;
         break;
         case 4:
           dval = tr.tracf;
         break;
         case 5:
           dval = tr.ep;
         break;
         case 6:
           dval = tr.cdp;
         break;
         case 7:
           dval = tr.cdpt;
         break;
         case 8:
           dval = tr.trid;
         break;
         case 9:
           dval = tr.nvs;
         break;
         case 10:
           dval = tr.nhs;
         break;
         case 11:
           dval = tr.duse;
         break;
         case 12:
           dval = tr.offset;
         break;
         case 13:
           dval = tr.gelev;
         break;
         case 14:
           dval = tr.selev;
         break;
         case 15:
           dval = tr.sdepth;
         break;
         case 16:
           dval = tr.gdel;
         break;
         case 17:
           dval = tr.sdel;
         break;
         case 18:
           dval = tr.swdep;
         break;
         case 19:
           dval = tr.gwdep;
         break;
         case 20:
           dval = tr.scalel;
         break;
         case 21:
           dval = tr.scalco;
         break;
         case 22:
           dval = tr.sx;
         break;
         case 23:
           dval = tr.sy;
         break;
         case 24:
           dval = tr.gx;
         break;
         case 25:
           dval = tr.gy;
         break;
         case 26:
           dval = tr.counit;
         break;
         case 27:
           dval = tr.wevel;
         break;
         case 28:
           dval = tr.swevel;
         break;
         case 29:
           dval = tr.sut;
         break;
         case 30:
           dval = tr.gut;
         break;
         case 31:
           dval = tr.sstat;
         break;
         case 32:
           dval = tr.gstat;
         break;
         case 33:
           dval = tr.tstat;
         break;
         case 34:
           dval = tr.laga;
         break;
         case 35:
           dval = tr.lagb;
         break;
         case 36:
           dval = tr.delrt;
         break;
         case 37:
           dval = tr.muts;
         break;
         case 38:
           dval = tr.mute;
         break;
         case 39:
           dval = tr.ns;
         break;
         case 40:
           dval = tr.dt;
         break;
         case 41:
           dval = tr.gain;
         break;
         case 42:
           dval = tr.igc;
         break;
         case 43:
           dval = tr.igi;
         break;
         case 44:
           dval = tr.corr;
         break;
         case 45:
           dval = tr.sfs;
         break;
         case 46:
           dval = tr.sfe;
         break;
         case 47:
           dval = tr.slen;
         break;
         case 48:
           dval = tr.styp;
         break;
         case 49:
           dval = tr.stas;
         break;
         case 50:
           dval = tr.stae;
         break;
         case 51:
           dval = tr.tatyp;
         break;
         case 52:
           dval = tr.afilf;
         break;
         case 53:
           dval = tr.afils;
         break;
         case 54:
           dval = tr.nofilf;
         break;
         case 55:
           dval = tr.nofils;
         break;
         case 56:
           dval = tr.lcf;
         break;
         case 57:
           dval = tr.hcf;
         break;
         case 58:
           dval = tr.lcs;
         break;
         case 59:
           dval = tr.hcs;
         break;
         case 60:
           dval = tr.year;
         break;
         case 61:
           dval = tr.day;
         break;
         case 62:
           dval = tr.hour;
         break;
         case 63:
           dval = tr.minute;
         break;
         case 64:
           dval = tr.sec;
         break;
         case 65:
           dval = tr.timbas;
         break;
         case 66:
           dval = tr.trwf;
         break;
         case 67:
           dval = tr.grnors;
         break;
         case 68:
           dval = tr.grnofr;
         break;
         case 69:
           dval = tr.grnlof;
         break;
         case 70:
           dval = tr.gaps;
         break;
         case 71:
           dval = tr.otrav;
         break;
         case 72:
           dval = tr.d1;
         break;
         case 73:
           dval = tr.f1;
         break;
         case 74:
           dval = tr.d2;
         break;
         case 75:
           dval = tr.f2;
         break;
         case 76:
           dval = tr.ungpow;
         break;
         case 77:
           dval = tr.unscale;
         break;
         case 78:
           dval = tr.ntr;
         break;
         case 79:
           dval = tr.mark;
         break;
         case 80:
           dval = tr.shortpad;
         break;
  
        } /* end of   switch */ 
        
      return (dval);
}
/* ---------------------------------------------------------------------------------- */
/* Set value into trace header using case number from GetCase.                        */
/*                                                                                    */
/* Input arguments:                                                                   */
/*     tr = a trace                                                                   */
/*     k  = the case number of the key name passed to GetCase.                        */
/*          If case number is not >0 and <81, nothing is set.                         */
/* dvalue = value to set in header for the case (key).                                */
/*                                                                                    */
/* Return: void                                                                       */
/*                                                                                    */
/* All integer key values are rounded to nearest integer before update (using lrint). */
/* There is no check to see if dvalue is actually small enough to fit the key type.   */
/* The raw input value is set (scalel and scalco flags are not considered).           */
/*                                                                                    */
void tohead(segy *tr, int k, double dvalue) {

       switch (k) {
  
         case -1: 
/*       null, name not found? */
         break;
         case 0:  
/*       null   do not write to header */ 
         break;
         case 1:
           tr->tracl = lrint(dvalue);  
         break;
         case 2:
           tr->tracr = lrint(dvalue);
         break;
         case 3:
           tr->fldr = lrint(dvalue);
         break;
         case 4:
           tr->tracf = lrint(dvalue);
         break;
         case 5:
           tr->ep = lrint(dvalue);
         break;
         case 6:
           tr->cdp = lrint(dvalue);
         break;
         case 7:
           tr->cdpt = lrint(dvalue);
         break;
         case 8:
           tr->trid = lrint(dvalue);
         break;
         case 9:
           tr->nvs = lrint(dvalue);
         break;
         case 10:
           tr->nhs = lrint(dvalue);
         break;
         case 11:
           tr->duse = lrint(dvalue);
         break;
         case 12:
           tr->offset = lrint(dvalue);
         break;
         case 13:
           tr->gelev = lrint(dvalue);
         break;
         case 14:
           tr->selev = lrint(dvalue);
         break;
         case 15:
           tr->sdepth = lrint(dvalue);
         break;
         case 16:
           tr->gdel = lrint(dvalue);
         break;
         case 17:
           tr->sdel = lrint(dvalue);
         break;
         case 18:
           tr->swdep = lrint(dvalue);
         break;
         case 19:
           tr->gwdep = lrint(dvalue);
         break;
         case 20:
           tr->scalel = lrint(dvalue);
         break;
         case 21:
           tr->scalco = lrint(dvalue);
         break;
         case 22:
           tr->sx = lrint(dvalue);
         break;
         case 23:
           tr->sy = lrint(dvalue);
         break;
         case 24:
           tr->gx = lrint(dvalue);
         break;
         case 25:
           tr->gy = lrint(dvalue);
         break;
         case 26:
           tr->counit = lrint(dvalue);
         break;
         case 27:
           tr->wevel = lrint(dvalue);
         break;
         case 28:
           tr->swevel = lrint(dvalue);
         break;
         case 29:
           tr->sut = lrint(dvalue);
         break;
         case 30:
           tr->gut = lrint(dvalue);
         break;
         case 31:
           tr->sstat = lrint(dvalue);
         break;
         case 32:
           tr->gstat = lrint(dvalue);
         break;
         case 33:
           tr->tstat = lrint(dvalue);
         break;
         case 34:
           tr->laga = lrint(dvalue);
         break;
         case 35:
           tr->lagb = lrint(dvalue);
         break;
         case 36:
           tr->delrt = lrint(dvalue);
         break;
         case 37:
           tr->muts = lrint(dvalue);
         break;
         case 38:
           tr->mute = lrint(dvalue);
         break;
         case 39:
           tr->ns = lrint(dvalue);
         break;
         case 40:
           tr->dt = lrint(dvalue);
         break;
         case 41:
           tr->gain = lrint(dvalue);
         break;
         case 42:
           tr->igc = lrint(dvalue);
         break;
         case 43:
           tr->igi = lrint(dvalue);
         break;
         case 44:
           tr->corr = lrint(dvalue);
         break;
         case 45:
           tr->sfs = lrint(dvalue);
         break;
         case 46:
           tr->sfe = lrint(dvalue);
         break;
         case 47:
           tr->slen = lrint(dvalue);
         break;
         case 48:
           tr->styp = lrint(dvalue);
         break;
         case 49:
           tr->stas = lrint(dvalue);
         break;
         case 50:
           tr->stae = lrint(dvalue);
         break;
         case 51:
           tr->tatyp = lrint(dvalue);
         break;
         case 52:
           tr->afilf = lrint(dvalue);
         break;
         case 53:
           tr->afils = lrint(dvalue);
         break;
         case 54:
           tr->nofilf = lrint(dvalue);
         break;
         case 55:
           tr->nofils = lrint(dvalue);
         break;
         case 56:
           tr->lcf = lrint(dvalue);
         break;
         case 57:
           tr->hcf = lrint(dvalue);
         break;
         case 58:
           tr->lcs = lrint(dvalue);
         break;
         case 59:
           tr->hcs = lrint(dvalue);
         break;
         case 60:
           tr->year = lrint(dvalue);
         break;
         case 61:
           tr->day = lrint(dvalue);
         break;
         case 62:
           tr->hour = lrint(dvalue);
         break;
         case 63:
           tr->minute = lrint(dvalue);
         break;
         case 64:
           tr->sec = lrint(dvalue);
         break;
         case 65:
           tr->timbas = lrint(dvalue);
         break;
         case 66:
           tr->trwf = lrint(dvalue);
         break;
         case 67:
           tr->grnors = lrint(dvalue);
         break;
         case 68:
           tr->grnofr = lrint(dvalue);
         break;
         case 69:
           tr->grnlof = lrint(dvalue);
         break;
         case 70:
           tr->gaps = lrint(dvalue);
         break;
         case 71:
           tr->otrav = lrint(dvalue);
         break;
         case 72:
           tr->d1 = dvalue;
         break;
         case 73:
           tr->f1 = dvalue;
         break;
         case 74:
           tr->d2 = dvalue;
         break;
         case 75:
           tr->f2 = dvalue;
         break;
         case 76:
           tr->ungpow = dvalue;
         break;
         case 77:
           tr->unscale = dvalue;
         break;
         case 78:
           tr->ntr = lrint(dvalue);
         break;
         case 79:
           tr->mark = lrint(dvalue);
         break;
         case 80:
           tr->shortpad = lrint(dvalue);
         break;

        } /* end of   switch                       */ 

}
