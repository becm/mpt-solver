# mpt-solver/modules/sundials/meson.build
# build definition for SUNDIALS module library

sundials_dir = get_option('sundials')

if not (sundials_dir == '')
  sundials_deps = [ cc.find_library('sundials_nvec' + get_option('sundials_vec'), dirs : sundials_dir + '/lib') ]
  
  sundials_ida = cc.find_library('sundials_ida', required : false, dirs : sundials_dir + '/lib')
  sundials_cvode = cc.find_library('sundials_cvode', required : false, dirs : sundials_dir + '/lib')
  sundials_kinsol = cc.find_library('sundials_kinsol', required : false, dirs : sundials_dir + '/lib')

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
  
  install_data(sources : 'sundials.conf', install_dir : 'etc/mpt.conf.d')
  
  mod_sundials = shared_library('mpt_sundials', sundials_src, 'libinfo.c',
    include_directories : [ include_directories(sundials_dir + '/include') ] + solv_mod_inc,
    install_rpath : sundials_dir + '/lib',
    dependencies : [ sundials_deps, cc.find_library('m') ],
    version : '1.0.0', soversion : '1',
    install : true)

endif
