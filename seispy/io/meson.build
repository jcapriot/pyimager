module_path = 'seispy/io'

io_args = []
# check for file io options:
if cc.has_function('fseeko', prefix: '#define _GNU_SOURCE\n#include <stdio.h>')
    io_args += ['-DHAVE_FSEEKO']
endif

if cc.has_function('ftello', prefix: '#define _GNU_SOURCE\n#include <stdio.h>')
    io_args += ['-DHAVE_FTELLO']
endif

# Check for byteswapping builtins
if cc.has_function('__builtin_bswap16')
    io_args += ['-DHAVE___BUILTIN_BSWAP16']
endif
if cc.has_function('__builtin_bswap32')
    io_args += ['-DHAVE___BUILTIN_BSWAP32']
endif
if cc.has_function('__builtin_bswap64')
    io_args += ['-DHAVE___BUILTIN_BSWAP64']
endif

py.extension_module(
    '__init__',
    '__init__.pyx',
    cython_args: cython_args,
    c_args: io_args,
    install: true,
    subdir: module_path,
    dependencies: [py_dep, np_dep],
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