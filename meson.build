# mpt-solver/meson.build
# build definitions for mpt solvers

project('mptsolver', 'c')

libs = get_option('libraries')
if libs == 'shared'
  with_static = false
  with_shared = true
elif libs == 'static'
  with_static = true
  with_shared = false
else
  with_static = true
  with_shared = true
endif

mpt_inc = get_option('prefix') + '/' + get_option('includedir') + '/mpt'
mpt_lib = get_option('prefix') + '/' + get_option('libdir')

core = find_library('mptcore', required : false, dirs : mpt_lib)
plot = find_library('mptplot', required : false, dirs : mpt_lib)

if core.found()
  if not plot.found()
    error('unusable mpt-base install state')
  endif
  inc = include_directories('.', mpt_inc)
else
  if plot.found()
    error('unusable mpt-base install state')
  endif
  subdir('base/mptcore')
  subdir('base/mptplot')
  inc = include_directories('.')
endif

install_headers('solver.h', subdir : 'mpt')

subdir('mptsolver')
