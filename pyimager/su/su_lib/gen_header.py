import argparse

parser = argparse.ArgumentParser(
                    prog='gen_segy_header',
                    description='Generates the header file describing the compile time size of the segy header'
)

parser.add_argument('SU_NKEYS', type=int)
parser.add_argument('HDRBYTES', type=int)
parser.add_argument('MAXSEGY', type=int)
parser.add_argument('output', type=str)

SU_NKEYS = parser.parse_args().SU_NKEYS
HDRBYTES = parser.parse_args().HDRBYTES
MAXSEGY = parser.parse_args().MAXSEGY
OUTFILE = parser.parse_args().output

header_h = f'''
/*
 * header.h - include file for segy sizes
 * THIS HEADER FILE IS GENERATED AUTOMATICALLY
 */
#ifndef HEADER_H
#define HEADER_H

#define SU_NKEYS {SU_NKEYS} /* Number of key header words */
#define HDRBYTES {HDRBYTES} /* Bytes in the trace header */
#define	MAXSEGY {MAXSEGY}

#endif
'''

with open(OUTFILE, 'w') as f:
    f.write(header_h)