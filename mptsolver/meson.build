# mpt-solver/mptsolver/meson.build
# build definition for mpt solver library

solv_src = files(
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
solv_inc = [ '..', '../base' ]

# add loader sources
if not load.found()
  src = []
  fname = '../base/mptloader/@0@'
  foreach f : [
    'proxy_type.c',
    'library_assign.c',
    'library_open.c',
    'meta_open.c',
    'library_bind.c',
    'readline.c'
  ]
    src += fname.format(f)
  endforeach
  solv_src += files(src)
  
  # need explicit dynamic loader library
  if host_machine.system() == 'linux'
    load = meson.get_compiler('c').find_library('dl')
  endif
endif

# add plot sources
if not plot.found()
  solv_src += plot_src
  solv_inc += '../base/mptplot'
endif

solv = shared_library('mptsolver', solv_src, 'libinfo.c',
  include_directories : include_directories(solv_inc),
  install_rpath : '$ORIGIN',
  dependencies : [ plot, load, core ],
  version : '1.0.0', soversion : '1',
  install : true)

pkg.generate(
  description : 'mpt solver library',
  libraries : solv,
  version : '1.0.0',
  name : 'mptsolver')
