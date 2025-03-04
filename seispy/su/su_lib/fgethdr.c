/* Copyright (c) Colorado School of Mines, 2011.*/
/* All rights reserved.                       */

/* FGETHR: $Revision: 1.2 $; $Date: 2011/11/11 23:57:38 $	*/

/*********************** self documentation **********************/
/****************************************************************** 
fgethdr - get segy tape identification headers from the file by file pointer
 
****************************************************************** 
Input:
fp         file pointer

Output:
chdr       3200 bytes of segy character header
bhdr        400 bytes of segy binary header
****************************************************************** 
Authors:  zhiming li  and j. dulac ,   unocal
 modified for CWP/SU: R. Beardsley
*******************************************************************/
/**************** end self doc ********************************/


#include "su.h"
#include "segy.h"
#include "bheader.h"


void fgethdr(FILE *fp, char* chdr, bhed* bhdr)
{
	efread(chdr, 1, EBCBYTES, fp);
	efread(bhdr, 1, sizeof(bhed), fp);
}
