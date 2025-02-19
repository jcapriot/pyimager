#include "su_filters.h"
#include "segy.h"
#include "cwp.h"

void bfhighpass_trace(
    int zerophase,
    int npoles,
    float f3db,
    int nt,
    segy *tr
    )
{

    bfhighpass(npoles,f3db,nt,tr->data,tr->data);
    if (zerophase) {
        register int i;
        for (i=0; i<nt/2; ++i) { /* reverse trace in place */
            register float tmp = tr->data[i];
            tr->data[i] = tr->data[nt-1 - i];
            tr->data[nt-1 - i] = tmp;
        }
        bfhighpass(npoles,f3db,nt,tr->data,tr->data);
        for (i=0; i<nt/2; ++i) { /* flip trace back */
            register float tmp = tr->data[i];
            tr->data[i] = tr->data[nt-1 - i];
            tr->data[nt-1 - i] = tmp;
        }
    }
}

void bflowpass_trace(
    int zerophase,
    int npoles,
    float f3db,
    int nt,
    segy *tr
    )
{

    bflowpass(npoles,f3db,nt,tr->data,tr->data);
    if (zerophase) {
        register int i;
        for (i=0; i<nt/2; ++i) { /* reverse trace */
            register float tmp = tr->data[i];
            tr->data[i] = tr->data[nt-1 - i];
            tr->data[nt-1 - i] = tmp;
        }
        bflowpass(npoles,f3db,nt,tr->data,tr->data);
        for (i=0; i<nt/2; ++i) { /* flip trace back */
            register float tmp = tr->data[i];
            tr->data[i] = tr->data[nt-1 - i];
            tr->data[nt-1 - i] = tmp;
        }
    }
}