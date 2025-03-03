module_path = 'seispy/io'

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

py.extension_module(
    '__init__',
    '__init__.pyx',
    cython_args: cython_args,
    c_args: io_args,
    install: true,
    subdir: module_path,
    dependencies: [py_dep],
)

py.extension_module(
    'segy_standard',
    'segy_standard.pyx',
    cython_args: cython_args,
    c_args: io_args,
    install: true,
    subdir: module_path,
    dependencies: [py_dep, np_dep],
)

python_sources = []

py.install_sources(
  python_sources,
  subdir: module_path
)