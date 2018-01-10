# mpt-solver/modules/sundials/meson.build
# build definition for SUNDIALS module library

modules_src_gen = files(
  'mod_valloc.c',
  'mod_value.c',
  join_paths('..', 'base', 'mptcore', 'meta', 'consume.c'),
  join_paths('..', 'base', 'mptcore', 'meta', 'consumable_setup.c')
)
modules_src_tol = files(
  'mod_tol_set.c',
  'mod_tol_get.c',
  'mod_tol_check.c'
)
modules_src_ivp = files('mod_ivpset.c', 'mod_nextval.c') + modules_src_gen + modules_src_tol
modules_dep_ode = declare_dependency(
  include_directories : solv_inc,
  sources : modules_src_ivp + files('mod_ufcn_ode.c')
)
modules_dep_dae = declare_dependency(
  include_directories : solv_inc,
  sources : modules_src_ivp + files('mod_ufcn_dae.c')
)

modules_src_nls = files('mod_nlsset.c') + modules_src_gen
modules_dep_nls = declare_dependency(
  include_directories : solv_inc,
  sources : modules_src_nls + files('mod_ufcn_nls.c')
)
modules_src_ufcn = files('mod_ufcn_ode.c', 'mod_ufcn_dae.c', 'mod_ufcn_nls.c')
modules_src =  modules_src_ivp + modules_src_ufcn + files('mod_nlsset.c')

sundials_dir = get_option('sundials')
if not (sundials_dir == '')
  subdir('sundials')
endif
