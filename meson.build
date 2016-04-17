# mpt-solver/meson.build
# build definitions for mpt solvers

project('mptsolver', 'c',
  default_options : [ 'c_std=c11' ]
)

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

mpt_lib = get_option('prefix') + '/' + get_option('libdir')
mpt_inc = get_option('prefix') + '/' + get_option('includedir') + '/mpt'

# find base libraries in target location
cc = meson.get_compiler('c')
core = cc.find_library('mptcore', required : false, dirs : mpt_lib)
plot = cc.find_library('mptplot', required : false, dirs : mpt_lib)
load = cc.find_library('mptloader', required : false, dirs : mpt_lib)

mpt = [ '.' ]
if core.found() or plot.found() or load.found()
  mpt += mpt_inc
endif
mpt = declare_dependency(include_directories : include_directories(mpt))

# build required libraries
with_core = not core.found()
if with_core
  subdir('base/mptcore')
endif

with_plot = not plot.found()
if with_plot
  subdir('base/mptplot')
endif

with_loader = not load.found()
if with_loader
  subdir('base/mptloader')
endif

# optional library for client
with_io = get_option('with_io')
if with_io
  io = cc.find_library('mptio', required : false, dirs : mpt_lib)
  if not io.found()
    subdir('base/mptio')
  endif
endif

install_headers('solver.h', subdir : 'mpt')

subdir('mptsolver')
subdir('modules/sundials')
