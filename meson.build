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


global = [ declare_dependency(include_directories : include_directories('.')) ]

core = find_library('mptcore', required : false, dirs : mpt_lib)
if not core.found()
  with_core = true
  subdir('base/mptcore')
  inc = include_directories('base/mptcore')
else
  inc = include_directories(mpt_inc)
  core = [ core, declare_dependency(include_directories : inc) ]
  global += declare_dependency(include_directories : inc)
endif

plot = find_library('mptplot', required : false, dirs : mpt_lib)
if not plot.found()
  with_plot = true
  subdir('base/mptplot')
else
  global += declare_dependency(include_directories : include_directories(mpt_inc))
endif

load = find_library('mptloader', required : false, dirs : mpt_lib)
if not load.found()
  with_loader = true
  subdir('base/mptloader')
endif

mpt = [ plot, core, load, global ]

install_headers('solver.h', subdir : 'mpt')

subdir('mptsolver')
