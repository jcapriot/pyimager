par_lib_src = [
    'atopkge.c',
     #'docpkge.c',
    'ealloc.c',
    'errpkge.c',
    # 'filestat.c',
    'fractal.c',
    'getpars.c',
    #'lincoeff.c',
    'minfunc.c',
    'modeling.c',
    'refaniso.c',
    'rke.c',
    'smooth.c',
    'subcalls.c',
    #'syscalls.c',
    'taup.c',
    'upweik.c',
    'VND.c',
    'wtlib.c',
]

par_include_dir = include_directories('include')

par_lib = library(
    'par',
    par_lib_src,
    include_directories: par_include_dir,
    dependencies: cwp_dep
)

par_dep = declare_dependency(
    include_directories: par_include_dir,
    link_with: par_lib,
    dependencies: cwp_dep
)