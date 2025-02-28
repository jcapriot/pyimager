filter_src = [
    'subfilt.c',
    #'succfilt.c',
    #'sudeghost.c',
    #'sudipfilt.c',
    #'sufilter.c',
    #'sufrac.c',
    #'sufwatrim.c',
    #'suk1k2filter.c',
    #'sukfilter.c',
    #'sukfrac.c',
    #'sulfaf.c',
    #'sumedian.c',
    #'suminphase.c',
    #'suphase.c',
    #'susmgauss2.c',
    #'susvdfilt.c',
    #'sutvband.c',
]

su_filter_include = include_directories('.')

su_filter_lib = library(
    'su_filters',
    filter_src,
    include_directories: su_filter_include,
    dependencies: [su_dep, cwp_dep],
)

su_filter_dep = declare_dependency(
    include_directories: su_filter_include,
    link_with: su_filter_lib,
    dependencies: [su_dep, cwp_dep],
)
