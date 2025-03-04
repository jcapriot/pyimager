#ifndef SUFILTERS_H
#define SUFILTERS_H
#include "spy_trace.h"

void bfhighpass_trace(int zerophase, int npoles, float f3db, spy_trace *tr_in, spy_trace *tr);
void bflowpass_trace(int zerophase, int npoles, float f3db, spy_trace *tr_in, spy_trace *tr);

#endif