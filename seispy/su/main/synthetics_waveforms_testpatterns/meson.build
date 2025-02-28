synthetics_src = [
    #'sukdsyn2d.c',
    'susynlv.c',
]

su_synthetics_include = include_directories('.')

su_synthetics_lib = library(
    'su_synthetics',
    synthetics_src,
    include_directories: su_synthetics_include,
    dependencies: [su_dep, par_dep, cwp_dep],
)

su_synthetics_dep = declare_dependency(
    include_directories: su_synthetics_include,
    link_with: su_synthetics_lib,
    dependencies: [su_dep, par_dep, cwp_dep],
)
