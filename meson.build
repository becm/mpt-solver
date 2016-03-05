# mpt-solver/meson.build
# build definitions for mpt solvers

project('mptsolver', 'c', default_options : [
  'c_std=c11'
])

pkg = import('pkgconfig')

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

global = [ declare_dependency(include_directories : include_directories('.')) ]

# find base libraries in target location
mpt_lib = get_option('prefix') + '/' + get_option('libdir')
core = find_library('mptcore', required : false, dirs : mpt_lib)
plot = find_library('mptplot', required : false, dirs : mpt_lib)
load = find_library('mptloader', required : false, dirs : mpt_lib)

if core.found() and plot.found() and load.found()
  # base libraries in target location
  mpt_inc = get_option('prefix') + '/' + get_option('includedir') + '/mpt'
  global += declare_dependency(include_directories : include_directories(mpt_inc))
else
  # build dependant libraries
  inc = include_directories('base')
  with_core = true
  subdir('base/mptcore')
  with_plot = true
  subdir('base/mptplot')
  with_loader = true
  subdir('base/mptloader')
  
  # select via meson options
  with_io = get_option('with_io')
  subdir('base/mptio')
endif

install_headers('solver.h', subdir : 'mpt')

mpt = [ plot, core, load, global ]

subdir('mptsolver')
subdir('modules/sundials')
