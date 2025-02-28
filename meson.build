project(
    'seispy',
    'c', 'cython',
    version : '0.0.1',
    license: 'MIT',
)

cc = meson.get_compiler('c')
cy = meson.get_compiler('cython')

py_mod = import('python')
py = py_mod.find_installation(pure: false)
py_dep = py.dependency()

m_dep = cc.find_library('m', required : false)
if m_dep.found()
  add_project_link_arguments('-lm', language : 'c')
endif

_global_c_args = cc.get_supported_arguments(
  '-Wno-unused-but-set-variable',
  '-Wno-unused-function',
  '-Wno-conversion',
  '-Wno-misleading-indentation',
)
add_project_arguments(_global_c_args, language : 'c')

subdir('seispy')