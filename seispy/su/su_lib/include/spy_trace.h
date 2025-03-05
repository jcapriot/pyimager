#ifndef _SPY_TRACE_H
#define _SPY_TRACE_H

#include <stddef.h> // size_t

typedef struct SPY_TRACE_HEADER{
    size_t n_sample;                // number of samples
    double d_sample;                // sample spacing  (seconds, hertz, meters)
    double sample_start;            // first sample  (seconds, hertz, meters)
    double offset;                  // along line distance between source and receiver (meters)
    double tx_loc[3];               // source location (East, North, Elevation) (meters)
    double rx_loc[3];               // receiver location (East, North, Elevation) (meters)
    double mid_point[3];            // source_receiver_midpoint (East, North, Elevation) (meters)
    size_t line_id;                 // line identifier
    size_t trace_id;                // trace identifier (along line)
    size_t ensemble_number;         // ensemble ID number
    size_t ensemble_trace_number;   // trace ID within ensemble
    unsigned int sampling_unit;     // 0 = s, 1  = meters
    unsigned int sampling_domain;   // 0 (sample unit domain), 1 = sample unit fourier domain

    float *data;                // samples
} spy_trace_header;

typedef struct SPY_TRACE{
    spy_trace_header hdr;   // The header container
    float *data;            // samples
} spy_trace;

#define SPY_TRC_SIZE sizeof(spy_trace)
#define SPY_TRC_HDR_SIZE sizeof(spy_trace_header)

#define SPY_SMPLNG_UNIT_SEC     0
#define SPY_SMPLNG_UNIT_METER   1

#define SPY_SMPLNG_DOM_UNIT     0
#define SPY_SMPLNG_DOM_FOURIER  1

#define SPY_UNKNOWN         0
#define SPY_TX_GATHER       1
#define SPY_RX_GATHER       2
#define SPY_COMMON_MIDPOINT 3
#define SPY_COMMON_OFFSET   4

#endif