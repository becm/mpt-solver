# mpt-solver/modules/sundials/meson.build
# build definition for SUNDIALS module library

dir_sundials = get_option('sundials')

if not (dir_sundials == '')
  sundials_deps = [find_library('sundials_nvec' + get_option('sundials_vec'), dirs : dir_sundials + '/lib')]
  
  ida = find_library('sundials_ida', required : false, dirs : dir_sundials + '/lib')
  cvode = find_library('sundials_cvode', required : false, dirs : dir_sundials + '/lib')
  kinsol = find_library('sundials_kinsol', required : false, dirs : dir_sundials + '/lib')

  src_lib = files(
    'ewtfcn.c', 'jacobian.c', 'nvector.c',
    'report.c', 'vector_set.c',
    '../ivppar_set.c',
    '../vecpar_settol.c',  '../vecpar_cktol.c',
    '../vecpar_value.c'
  )
  
  if ida.found()
    subdir('ida')
    src_lib += src
    sundials_deps += ida
  endif
  
  if cvode.found()
    subdir('cvode')
    src_lib += src
    sundials_deps += cvode
  endif
  
  mod_sundials = shared_library('mpt_sundials', src_lib, 'libinfo.c',
    include_directories : include_directories(dir_sundials + '/include', '..', '../..', '../../base', '../../base/mptcore'),
    install_rpath : dir_sundials + '/lib',
    dependencies : [ sundials_deps, find_library('m') ],
    version : '1.0.0', soversion : '1',
    install : true)

endif
