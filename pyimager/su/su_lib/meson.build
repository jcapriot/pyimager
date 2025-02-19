su_lib_srcs = [
    'bilinear.c',
    'elco_scalar.c',
    'fgetgthr.c',
    'fgethdr.c',
    'fgettr.c',
    'fputgthr.c',
    'fputtr.c',
    'getSPSfield.c',
    'gridread.c',
    'gridxy.c',
    'hdrpkge.c',
    'headcase.c',
    'linterpd.c',
    'qdefine.c',
    'readkfile.c',
    'tabplot.c',
    'valpkge.c',
    'xdrbhdrsub.c',
    'xdrhdrsub.c',
]

su_include_dir = include_directories('include')

su_lib = library(
    'su_lib',
    su_lib_srcs,
    include_directories: su_include_dir,
    dependencies: [cwp_dep, par_dep],
)

su_dep = declare_dependency(
    include_directories: su_include_dir,
    link_with: su_lib,
)