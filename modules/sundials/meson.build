# mpt-solver/modules/sundials/meson.build
# build definition for SUNDIALS module library

sundials_dir = get_option('sundials')

if not (sundials_dir == '')
  sundials_dir_lib = join_paths(sundials_dir, 'lib')
  sundials_inc = include_directories(join_paths(sundials_dir, 'include'))
  
  sundials_deps = [ cc.find_library('sundials_nvec' + get_option('sundials_vec'), dirs : sundials_dir_lib) ]
  
  sundials_ida = cc.find_library('sundials_ida', required : false, dirs : sundials_dir_lib)
  sundials_cvode = cc.find_library('sundials_cvode', required : false, dirs : sundials_dir_lib)
  sundials_kinsol = cc.find_library('sundials_kinsol', required : false, dirs : sundials_dir_lib)
  
  sundials_deps += declare_dependency(include_directories : sundials_inc)
  
  sundials_src = files(
    'ewtfcn.c', 'jacobian.c', 'nvector.c',
    'report.c', 'vector_set.c',
    '../ivppar_set.c',
    '../vecpar_settol.c',  '../vecpar_cktol.c',
    '../vecpar_value.c'
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
  
  sundials_deps += cc.find_library('m')
  
  install_data(sources : 'sundials.conf', install_dir : 'etc/mpt.conf.d')
  
  sundials_lib = shared_library('mpt_sundials', sundials_src, 'libinfo.c',
    include_directories : solv_mod_inc + [ sundials_inc ],
    link_args : '-Wl,-rpath,' + sundials_dir + '/lib',
    install_rpath : sundials_dir + '/lib',
    dependencies : sundials_deps,
    version : '1.0.0', soversion : '1',
    install : true)
  
  mod_sundials = declare_dependency(
    link_with : sundials_lib,
    link_args : '-Wl,-rpath,' + sundials_dir_lib,
    dependencies : sundials_deps,
    include_directories : [ sundials_inc, include_directories('.') ])

endif
