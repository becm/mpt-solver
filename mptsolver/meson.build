# mpt-solver/mptsolver/meson.build
# build definition for mpt solver library

solv_src = files(
  'client_ivp.c', 'client_nls.c',
  'conf_nls.c', 'conf_ode.c', 'conf_pde.c',
  'init_ivp.c', 'init_pde.c', 'init_nls.c',
  'steps_ode.c',
  'timeradd.c',
  'clientdata.c'
  'data_finit.c', 'data_grid.c', 'data_param.c', 'data_nls.c',
  'output_nls.c', 'output_pde.c',
  'solver_alias.c', 'solver_assign.c', 'solver_events.c', 'solver_load.c', 'solver_param.c',
  'solver_print.c', 'solver_pset.c', 'solver_read.c',
  'solver_start.c', 'solver_statistics.c',
  'residuals_cdiff.c'
)
solv_req = 'mptloader'

# add plot elements
if with_plot
  solv_src += plot_src
  solv_req += ' mptcore'
else
  solv_req += ' mptplot'
endif

if with_shared
  solv_lib = shared_library('mptsolver', solv_src, 'libinfo.c',
    include_directories : solv_inc,
    install_rpath : '$ORIGIN',
    dependencies : [ plot, load, core ],
    version : '1.0.0', soversion : '1',
    install : true)

  pkg.generate(
    description : 'mpt solver library',
    libraries : solv_lib,
    requires_private : solv_req,
    version : '1.0.0',
    name : 'mptsolver')

  solv = declare_dependency(
    link_with : solv_lib,
    include_directories : solv_inc
  )
endif
