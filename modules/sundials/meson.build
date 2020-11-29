# mpt-solver/modules/sundials/meson.build
# build definition for SUNDIALS module library

sundials_dir_lib = join_paths(sundials_dir, 'lib')
sundials_dir_inc = include_directories(join_paths(sundials_dir, 'include'))

sundials_deps = [ cc.find_library('sundials_nvec' + get_option('sundials_vec'), dirs : sundials_dir_lib) ]
sundials_args = []
sundials_ida = cc.find_library('sundials_ida', required : false, dirs : sundials_dir_lib)
sundials_cvode = cc.find_library('sundials_cvode', required : false, dirs : sundials_dir_lib)
sundials_kinsol = cc.find_library('sundials_kinsol', required : false, dirs : sundials_dir_lib)

sundials_deps += declare_dependency(include_directories : sundials_dir_inc)

sundials_src = files(
  'sundials_data_new.c',
  'sundials_ewtfcn.c',
  'sundials_finit.c',
  'sundials_jacobian.c',
  'sundials_linear.c',
  'sundials_modfcn.c',
  'sundials_nvector.c',
  'sundials_report.c'
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

if get_option('sundials_lapack')
  sundials_deps += cc.find_library('sundials_sunlinsollapackband', dirs : sundials_dir_lib)
  sundials_deps += cc.find_library('sundials_sunlinsollapackdense', dirs : sundials_dir_lib)
  sundials_args += '-DMPT_WITH_LAPACK'
endif

sundials_deps += libm

install_data(sources : 'sundials.conf', install_dir : join_paths('etc', 'mpt.conf.d'))

sundials_lib = shared_library('mpt_sundials', sundials_src, 'libinfo.c',
  include_directories : [ solv_mod_inc ],
  c_args : sundials_args,
  install_rpath : '$ORIGIN' + ':' + sundials_dir_lib,
  dependencies : [ sundials_deps, modules_dep_dae, modules_dep_ode ],
  version : '1.0.0', soversion : '1',
  install : true)

mod_sundials = declare_dependency(
  link_with : sundials_lib,
  dependencies : sundials_deps,
  include_directories : [ include_directories('.') ])
  
solv_mod_lib += [ meson.current_build_dir(), sundials_dir_lib ]
