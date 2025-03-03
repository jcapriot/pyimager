from libc.stdint cimport (
    uint8_t as ui1,\
    uint16_t as ui2,
    int16_t as i2,
    int32_t as i4,
    uint32_t as ui4,
    uint64_t as ui8,
)
from numpy cimport float64_t as r8

# constants to be defined in segy_standard.pyx
cdef:
    # Expected Sizes
    size_t TXT_HDR_SIZE
    size_t BIN_HDR_SIZE
    size_t TRC_HDR_SIZE

    i2 UNKNOWN

    # data_format codes
    i2 DAT_F32_IBM
    i2 DAT_I32
    i2 DAT_I16
    i2 DAT_F32_FGN
    # rev 1
    i2 DAT_F32_I3E
    i2 DAT_F64_I3E
    i2 DAT_I24
    i2 DAT_I08
    # rev 2
    i2 DAT_I64
    i2 DAT_U32
    i2 DAT_U16
    i2 DAT_U64
    i2 DAT_U24
    i2 DAT_U08

    # trace sorting codes
    i2 SORT_NONE
    i2 SORT_CMN_DP_PT
    i2 SORT_SINGLE
    i2 SORT_HORIZ_STK
    # rev 1
    i2 SORT_OTHER
    i2 SORT_CMN_SRC_PT
    i2 SORT_CMN_RX_PT
    i2 SORT_CMN_OFF_PT
    i2 SORT_CMN_MD_PT
    i2 SORT_CMN_CNV_PT

    # Sweep type code
    ui2 SWP_LINEAR
    ui2 SWP_PARABOLIC
    ui2 SWP_EXP
    ui2 SWP_OTHER
    ui2 TPR_LINEAR
    ui2 TPR_COS
    ui2 TPR_OTHER

    i2 CORR_DATA_YES
    i2 CORR_DATA_NO

    i2 BIN_GAIN_YES
    i2 BIN_GAIN_NO

    i2 AMP_REC_NONE
    i2 AMP_REC_SPH
    i2 AMP_REC_AGC
    i2 AMP_REC_OTHER

    i2 MEASURE_METERS
    i2 MEASURE_FEET

    i2 POLARITY_UP_NEG
    i2 POLARITY_UP_POS

    i2 VIB_POL_338_023
    i2 VIB_POL_023_068
    i2 VIB_POL_067_113
    i2 VIB_POL_113_158
    i2 VIB_POL_158_203
    i2 VIB_POL_203_248
    i2 VIB_POL_248_293
    i2 VIB_POL_293_338

    i4 ENDIAN_CORRECT
    i4 ENDIAN_REV
    i4 ENDIAN_PAIR_SWAP

    #Time codes, rev 2
    i2 TIME_LOCAL
    i2 TIME_GMT
    i2 TIME_OTHER
    i2 TIME_UTC
    i2 TIME_GPS

    # Survey Types, rev 2.1
    i2 SRV_LAND
    i2 SRV_MARINE
    i2 SRV_TRANS
    i2 SRV_DWN_HOLE
    i2 SRV_1D
    i2 SRV_2D
    i2 SRV_3D
    i2 SRV_TIME_LAPSE
    i2 SRV_PARALLEL_LINES
    i2 SRV_CRS_SPREAD
    i2 SRV_PATCHES
    i2 SRV_TWD_STRMR
    i2 SRV_OBS
    i2 SRV_RAND


cdef packed struct binary_header:
    i4 job_id
    i4 line_number
    i4 reel_number
    ui2 n_trace_per_ensemble
    ui2 n_aux_trace_per_ensemble
    ui2 d_sample # in \mu s, Hz, m, or ft.
    ui2 d_sample_field # in \mu s, Hz, m, or ft.
    ui2 n_sample_per_trace
    ui2 n_sample_per_trace_field
    i2 data_format
    ui2 n_fold
    i2 sort_method
    i2 vert_sum_code
    ui2 sweep_freq_start
    ui2 sweep_freq_end
    ui2 sweep_dur # in ms
    i2 sweep_type
    ui2 sweep_trace
    ui2 sweep_start_taper_dur # in ms
    ui2 sweep_end_taper_dur # in ms
    i2 taper_type
    i2 traces_are_correlated
    i2 bin_gain_is_recovered
    i2 amp_recovery_method
    i2 meters_or_feet
    i2 signal_polarity
    i2 vib_polarity_code
    # rev 2 (up to unassigned_1)
    ui4 next_trace_per_ensemble
    ui4 next_aux_trace_per_ensemble
    ui4 next_sample_per_trace
    r8 dext_sample # in \mu s, Hz, m, or ft.
    r8 dext_sample_field # in \mu s, Hz, m, or ft.
    ui4 next_sample_per_trace_field
    ui4 next_fold
    i4 byte_order_id
    ui1[200] unassigned_1
    # rev 1
    ui1 major_rev
    ui1 minor_rev
    ui2 is_fixed_traces
    ui2 n_ext_txt_hdr
    # rev 2 (up to unassigned_2)
    ui2 n_max_add_trc_hdr # rev 2.1 (In rev1 this is a ui4, and there is no survey type
    i2 survey_type # rev 2.1
    i2 time_basis
    ui8 n_traces
    ui8 first_trace_byte_offset
    ui4 n_trailer_stanzas
    ui1[68] unassigned_2


cdef packed struct trace_header:
    # rev 0
    i4 tracl
    i4 tracr
    i4 fldr
    i4 tracf
    i4 ep
    i4 cdp
    i2 cdpt
    i2 trid
    ui2 nvs
    ui2 nhs
    i2 duse
    i4 offset
    i4 gelev
    i4 selev
    i4 sdepth
    i4 gdel
    ui4 sdel
    ui4 gwdep
    i2 scalel
    i2 scalco
    i4 sx
    i4 sy
    i4 gx
    i4 gy
    i2 counit
    i2 wevel
    i2 swevel
    i2 sut
    i2 gut
    i2 sstat
    i2 gstat
    i2 tstat
    i2 laga
    i2 lagb
    i2 delrt
    i2 muts
    i2 mute
    ui2 ns
    ui2 dt
    i2 gain
    i2 igc
    i2 igi
    i2 corr
    ui2 sfs
    ui2 sfe
    ui2 slen # in ms
    i2 styp
    ui2 stas # in ms
    ui2 stae # in ms
    i2 tatyp
    ui2 afilf
    i2 afils
    ui2 nofilf
    i2 nofils
    ui2 lcf
    ui2 hcf
    i2 lcs
    i2 hcs
    ui2 year
    ui2 day
    ui2 hour
    ui2 minute
    ui2 second
    i2 timbas
    i2 trwf
    i2 grnors
    i2 grnofr
    i2 grnlof
    i2 gaps
    i2 otrav
    # rev 1
    i4 ens_pos_x
    i4 ens_pos_y
    i4 inline_num
    i4 xline_num
    i4 shot_point_num
    i2 shot_point_scale
    i2 data_units
    i4 transducer_mantis
    i2 transducer_pow10
    i2 transducer_units
    i2 device_trace_id
    i2 times_scale # to be applied to give true time in milliseconds of bytes 95-114
    i2 src_type
    i2 src_dir_vert
    i2 src_dir_xline
    i2 src_dir_inline
    i4 src_effort_mantis
    i2 src_effort_scale
    i2 src_units
    char[8] end




