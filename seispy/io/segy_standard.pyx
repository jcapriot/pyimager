from libc.string cimport memset, memcpy

cdef:
    # Expected Sizes
    size_t TXT_HDR_SIZE = 3200
    size_t BIN_HDR_SIZE = 400
    size_t TRC_HDR_SIZE = 240

    i2 UNKNOWN = 0

    # data_format codes
    i2 DAT_F32_IBM = 1
    i2 DAT_I32 = 2
    i2 DAT_I16 = 3
    i2 DAT_F32_FGN = 4
    i2 DAT_F32_I3E = 5
    i2 DAT_F64_I3E = 6
    i2 DAT_I24 = 7
    i2 DAT_I08 = 8
    i2 DAT_I64 = 9
    i2 DAT_U32 = 10
    i2 DAT_U16 = 11
    i2 DAT_U64 = 12
    i2 DAT_U24 = 15
    i2 DAT_U08 = 16

    # trace sorting codes
    i2 SORT_NONE = 1
    i2 SORT_CMN_DP_PT = 2
    i2 SORT_SINGLE = 3
    i2 SORT_HORIZ_STK = 4
    # rev 1
    i2 SORT_OTHER = -1
    i2 SORT_CMN_SRC_PT = 5
    i2 SORT_CMN_RX_PT = 6
    i2 SORT_CMN_OFF_PT = 7
    i2 SORT_CMN_MD_PT = 8
    i2 SORT_CMN_CNV_PT = 9

    # Sweep type code
    ui2 SWP_LINEAR = 1
    ui2 SWP_PARABOLIC = 2
    ui2 SWP_EXP = 3
    ui2 SWP_OTHER = 4
    ui2 TPR_LINEAR = 1
    ui2 TPR_COS = 2
    ui2 TPR_OTHER = 3

    i2 CORR_DATA_YES = 2
    i2 CORR_DATA_NO = 1

    i2 BIN_GAIN_YES = 1
    i2 BIN_GAIN_NO = 2

    i2 AMP_REC_NONE = 1
    i2 AMP_REC_SPH = 2
    i2 AMP_REC_AGC = 3
    i2 AMP_REC_OTHER = 4

    i2 MEASURE_METERS = 1
    i2 MEASURE_FEET = 2

    i2 POLARITY_UP_NEG = 1
    i2 POLARITY_UP_POS = 2

    i2 VIB_POL_338_023 = 1
    i2 VIB_POL_023_068 = 2
    i2 VIB_POL_067_113 = 3
    i2 VIB_POL_113_158 = 4
    i2 VIB_POL_158_203 = 5
    i2 VIB_POL_203_248 = 6
    i2 VIB_POL_248_293 = 7
    i2 VIB_POL_293_338 = 8

    i4 BIG_ENDIAN = 0
    i4 LITTLE_ENDIAN = 0
    i4 PAIRWISE_BYTESWAP = 0

    #Time codes, rev 2
    i2 TIME_LOCAL = 1
    i2 TIME_GMT = 2
    i2 TIME_OTHER = 3
    i2 TIME_UTC = 4
    i2 TIME_GPS = 5

    # Survey Types, rev 2.1
    i2 SRV_LAND = 1
    i2 SRV_MARINE = 2
    i2 SRV_TRANS = 3
    i2 SRV_DWN_HOLE = 4
    i2 SRV_1D= 8
    i2 SRV_2D = 16
    i2 SRV_3D = 24
    i2 SRV_TIME_LAPSE = 32
    i2 SRV_PARALLEL_LINES = 128
    i2 SRV_CRS_SPREAD = 256
    i2 SRV_PATCHES = 684
    i2 SRV_TWD_STRMR = 1024
    i2 SRV_OBS = 1152
    i2 SRV_RAND = 1280


cdef:
    ui1[4] _big = [1, 2, 3, 4]
    ui1[4] _small = [4, 3, 2, 1]
    ui1[4] _swapped = [2, 1, 4, 3]
memcpy(&BIG_ENDIAN, &_big[0], 4)
memcpy(&LITTLE_ENDIAN, &_small[0], 4)
memcpy(&PAIRWISE_BYTESWAP, &_swapped[0], 4)

print(BIG_ENDIAN, LITTLE_ENDIAN, PAIRWISE_BYTESWAP)

    # Survey Types

if sizeof(binary_header) != BIN_HDR_SIZE:
    raise Exception(
        f"Incorrect compiled binary segy header size: {sizeof(binary_header)}, expected {BIN_HDR_SIZE}."
    )

if sizeof(trace_header) != TRC_HDR_SIZE:
    raise Exception(
        f"Incorrect compiled segy trace header size: {sizeof(trace_header)}, expected {TRC_HDR_SIZE}."
    )


cdef class SEGYBinaryHeader:
    cdef:
        binary_header hdr


    def __cinit__(self):
        cdef binary_header *hdr = &self.hdr
        # initialize everything to 0
        memset(hdr, 0, BIN_HDR_SIZE)

    cpdef binary_header get_c_hdr(self):
        return self.hdr