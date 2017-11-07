# mpt-solver/modules/sundials/meson.build
# build definition for SUNDIALS module library

sundials_dir = get_option('sundials')

if not (sundials_dir == '')
  sundials_dir_lib = join_paths(sundials_dir, 'lib')
  sundials_dir_inc = include_directories(join_paths(sundials_dir, 'include'))
  
  sundials_deps = [ cc.find_library('sundials_nvec' + get_option('sundials_vec'), dirs : sundials_dir_lib) ]
  
  sundials_ida = cc.find_library('sundials_ida', required : false, dirs : sundials_dir_lib)
  sundials_cvode = cc.find_library('sundials_cvode', required : false, dirs : sundials_dir_lib)
  sundials_kinsol = cc.find_library('sundials_kinsol', required : false, dirs : sundials_dir_lib)
  
  sundials_deps += declare_dependency(include_directories : sundials_dir_inc)
  
  sundials_src = files(
    'data_new.c', 'ewtfcn.c', 'jacobian.c', 'nvector.c', 'report.c',
    'sundials_modfcn.c',
    '../mod_generic_conv.c',
    '../mod_ivpset.c',
    '../mod_nextval.c',
    '../mod_tol_set.c', '../mod_tol_get.c', '../mod_tol_check.c',
    '../mod_ufcn_dae.c', '../mod_ufcn_ode.c'
  )
  
  if sundials_ida.found()
    subdir('ida')
    sundials_src += src
    sundials_deps += sundials_ida
  endif
  
  if sundials_cvode.found()
    subdir('cvode')
    sundials_src += src
    sundials_deps += sundials_cvode
  endif
  
  sundials_deps += libm
  
  install_data(sources : 'sundials.conf', install_dir : join_paths('etc', 'mpt.conf.d'))
  
  sundials_lib = shared_library('mpt_sundials', sundials_src, 'libinfo.c',
    include_directories : solv_mod_inc,
    link_args : '-Wl,-rpath,' + sundials_dir_lib,
    install_rpath : sundials_dir_lib,
    dependencies : sundials_deps,
    version : '1.0.0', soversion : '1',
    install : true)
  
  mod_sundials = declare_dependency(
    link_with : sundials_lib,
    link_args : '-Wl,-rpath,' + sundials_dir_lib,
    dependencies : sundials_deps,
    include_directories : [ include_directories('.') ])

endif
