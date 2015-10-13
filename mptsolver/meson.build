# mpt-solver/mptsolver/meson.build
# build definition for mpt solver library

lib_src = files(
  'client_ivp.c', 'client_nls.c',
  'conf_nls.c', 'conf_ode.c', 'conf_pde.c',
  'data_finit.c', 'data_grid.c', 'data_param.c', 'data_timeradd.c',
  'residuals_cdiff.c',
  'solver_alias.c', 'solver_check.c', 'solver_events.c', 'solver_param.c',
  'solver_print.c', 'solver_pset.c', 'solver_read.c',
  'solver_start.c', 'solver_statistics.c',
  'step_nls.c', 'step_ode.c', 'step_pde.c'
)

lib = shared_library('mptsolver', lib_src, #'libinfo.c',
  include_directories : inc,
  install_rpath : '$ORIGIN',
  dependencies : [ plot, core ],
  version : '1.0.0', soversion : '1',
  install : true)

pkgconfig_gen(
  description : 'mpt solver library',
  libraries : lib,
  version : '1.0.0',
  name : 'mptsolver')


