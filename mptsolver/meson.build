# mpt-solver/mptsolver/meson.build
# build definition for mpt solver library

solv_src = files(
  'client_ivp.c', 'client_nls.c',
  'init_ivp.c', 'init_pde.c', 'init_nls.c',
  'residuals_cdiff.c',
  'solver_alias.c', 'solver_config.c',
  'solver_events.c', 'solver_load.c',
  'solver_param.c', 'solver_print.c', 'solver_pset.c',
  'solver_read.c',
  'solver_start.c', 'solver_statistics.c',
  'steps_ode.c',
  'timeradd.c'
)
foreach d : [ 'conf', 'data', 'output' ]
  subdir(d)
  solv_src += src
endforeach

# add plot elements
if not with_plot
  plot_src = []
endif

if with_shared
  solv_lib = shared_library('mptsolver', solv_src + plot_src, 'libinfo.c',
    include_directories : solv_inc,
    install_rpath : '$ORIGIN',
    dependencies : [ plot, load, core ],
    version : '1.0.0', soversion : '1',
    install : true)
endif

if with_static
  solv_lib_static = static_library('mptsolver', solv_src,
    include_directories : solv_inc_static,
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
  include_directories : solv_inc
)
