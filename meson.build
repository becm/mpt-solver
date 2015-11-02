# mpt-solver/meson.build
# build definitions for mpt solvers

project('mptsolver', 'c', default_options : [
  'c_std=c11'
])

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
  mpt_inc = get_option('prefix') + '/' + get_option('includedir') + '/mpt'
  global += declare_dependency(include_directories : include_directories(mpt_inc))

  # only install header when base is present in same location
  install_headers('solver.h', subdir : 'mpt')
else
  core = dependency('mptcore')
  plot = dependency('mptplot')
  load = dependency('mptloader')
  global += dependency('mpt-local-headers')
endif

mpt = [ plot, core, load, global ]

subdir('mptsolver')
