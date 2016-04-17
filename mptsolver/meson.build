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
solv_add = []

solv_reqires = 'mptcore'

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
  solv_add += files(src)
  
  # need explicit dynamic loader library
  if with_shared and host_machine.system() == 'linux'
    load = meson.get_compiler('c').find_library('dl')
  endif
else
  solv_reqires = 'mptloader ' + solv_reqires
endif

# add plot sources
if not plot.found()
  solv_add += plot_src
  solv_inc += '../base/mptplot'
else
  solv_reqires = 'mptplot ' + solv_reqires
endif

if with_shared
  libsolv = shared_library('mptsolver', solv_src, solv_add, 'libinfo.c',
    include_directories : include_directories(solv_inc),
    install_rpath : '$ORIGIN',
    dependencies : [ plot, load, core ],
    version : '1.0.0', soversion : '1',
    install : true)

  pkg.generate(
    description : 'mpt solver library',
    libraries : libsolv,
    requires : solv_reqires,
    version : '1.0.0',
    name : 'mptsolver')

  solv = declare_dependency(link_with : libsolv,
    include_directories : include_directories(solv_inc))
endif
