# mpt-solver/mptsolver/meson.build
# build definition for mpt solver library

src_lib = files(
  'client_ivp.c', 'client_nls.c',
  'conf_nls.c', 'conf_ode.c', 'conf_pde.c',
  'init_ivp.c', 'init_pde.c', 'init_nls.c',
  'steps_ode.c',
  'timeradd.c',
  'data_finit.c', 'data_grid.c', 'data_param.c', 'data_nls.c',
  'output_nls.c', 'output_pde.c',
  'solver_alias.c', 'solver_assign.c', 'solver_events.c', 'solver_load.c', 'solver_param.c',
  'solver_print.c', 'solver_pset.c', 'solver_read.c',
  'solver_start.c', 'solver_statistics.c',
  'residuals_cdiff.c'
)

lib = shared_library('mptsolver', src_lib, 'libinfo.c',
  include_directories : include_directories('../base'),
  install_rpath : '$ORIGIN',
  dependencies : mpt,
  version : '1.0.0', soversion : '1',
  install : true)

pkg.generate(
  description : 'mpt solver library',
  libraries : lib,
  version : '1.0.0',
  name : 'mptsolver')
