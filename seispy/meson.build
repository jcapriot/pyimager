subdir('su')

# NumPy include directory
numpy_nodepr_api = ['-DNPY_NO_DEPRECATED_API=NPY_1_22_API_VERSION']
np_dep = dependency('numpy')


# check for file io options:
have_fseeko = cc.has_function('fseeko', prefix: '#define _GNU_SOURCE\n#include <stdio.h>')
have_ftello = cc.has_function('ftello', prefix: '#define _GNU_SOURCE\n#include <stdio.h>')

io_args = []
if have_fseeko
    io_args += ['-DHAVE_FSEEKO']
endif
if have_ftello
    io_args += ['-DHAVE_FTELLO']
endif

# Deal with M_PI & friends; add `use_math_defines` to c_args or cpp_args
# Cython doesn't always get this right itself (see, e.g., gh-16800), so
# explicitly add the define as a compiler flag for Cython-generated code.
is_windows = host_machine.system() == 'windows'
if is_windows
  use_math_defines = ['-D_USE_MATH_DEFINES']
else
  use_math_defines = []
endif

c_undefined_ok = ['-Wno-maybe-uninitialized']

cython_c_args = [numpy_nodepr_api, use_math_defines, '-DCYTHON_TRACE_NOGIL=1']

cython_args = []
if cy.version().version_compare('>=3.1.0')
  cython_args += ['-Xfreethreading_compatible=True']
endif


python_sources = [
    '__init__.py',
    'plotting.py',
]

module_path = 'seispy'

py.install_sources(
  python_sources,
  subdir: module_path
)

py.extension_module(
    '_io',
    '_io.pyx',
    cython_args: cython_args,
    c_args: io_args,
    install: true,
    subdir: module_path,
    dependencies: [py_dep],
)

py.extension_module(
    'segy',
    'segy.pyx',
    cython_args: cython_args,
    c_args: cython_c_args,
    install: true,
    subdir: module_path,
    dependencies : [py_dep, su_dep, cwp_dep, par_dep],
)

py.extension_module(
    'synthetics',
    'synthetics.pyx',
    cython_args: cython_args,
    c_args: cython_c_args,
    install: true,
    subdir: module_path,
    dependencies : [py_dep, su_synthetics_dep],
)

py.extension_module(
    'filters',
    'filters.pyx',
    cython_args: cython_args,
    c_args: cython_c_args,
    install: true,
    subdir: module_path,
    dependencies : [py_dep, su_filter_dep, su_dep, cwp_dep, par_dep],
)