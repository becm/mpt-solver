# mpt-solver/meson.build
# build definitions for mpt solver components

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

# require core library
with_core = not core.found()
if with_core
  subdir('base/mptcore')
  solv_inc_static = [ 'base/mptcore' ]
else
  solv_inc_static = [ mpt_inc ]
  core = [ core, declare_dependency(include_directories : include_directories(mpt_inc)) ]
endif

# require loader library
with_loader = not load.found()
if with_loader
  subdir('base/mptloader')
else
  load = [ load, declare_dependency(include_directories : include_directories(mpt_inc)) ]
endif

# query required plot sources
with_plot = not plot.found()
if with_plot
  plot_src = []
  foreach d : [ 'base/mptplot/output', 'base/mptplot/values' ]
    subdir(d)
    plot_src += src
  endforeach
  solv_inc_static += 'base/mptplot'
  plot = [ core, declare_dependency(include_directories : include_directories('base/mptplot')) ]
else
  # need target include path addition
  plot = [ plot, declare_dependency(include_directories : include_directories(mpt_inc)) ]
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

# solver library creation
solv_inc = include_directories('.', 'base')
solv_inc_static = [ solv_inc, include_directories(solv_inc_static) ]

subdir('mptsolver')

# solver modules creation
solv_mod_inc = [ solv_inc, include_directories('modules') ]
if with_core
  solv_mod_inc += core_inc
else
  solv_mod_inc += include_directories(mpt_inc)
endif
subdir('modules/sundials')
