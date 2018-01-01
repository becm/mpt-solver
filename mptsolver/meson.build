# mpt-solver/mptsolver/meson.build
# build definition for mpt solver library

solv_src = files(
  'conf_graphic.c', 'conf_history.c', 'conf_param.c',
  'output_solver_history.c', 'output_solver_data.c',
  'solver_alias.c',
  'solver_data_fini.c', 'solver_dispatch.c',
  'solver_load.c',
  'solver_output_query.c',
  'solver_param.c', 'solver_print.c',
  'solver_read.c', 'solver_require.c',
  'solver_statistics.c',
  'solver_typeid.c',
  'timeradd.c'
)
foreach d : [ 'ivp', 'nls' ]
  subdir(d)
  solv_src += src
endforeach

if with_shared
  solv_dep = [ plot, load, core ]
  solv_lib = shared_library('mptsolver', solv_src, 'libinfo.c',
    include_directories : solv_inc,
    install_rpath : '$ORIGIN',
    dependencies : solv_dep,
    version : '1.0.0', soversion : '1',
    install : true)
endif

if with_static
  solv_lib_static = static_library('mptsolver', solv_src, modules_src,
    include_directories : solv_inc,
    dependencies : solv_dep,
    install : true)
  if not with_shared
    solv_lib = solv_lib_static
  endif
endif

pkg.generate(
  description : 'mpt solver library',
  libraries : solv_lib,
  requires_private : 'mpt',
  version : '1.0.0',
  name : 'mptsolver')

solv = declare_dependency(
  link_with : solv_lib,
  dependencies : solv_dep,
  include_directories : solv_inc
)
