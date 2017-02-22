# mpt-solver/meson.build
# build definitions for mpt solver components

project('mptsolver', 'c',
  default_options : [ 'c_std=c11' ]
)

pkg = import('pkgconfig')

mpt_lib = join_paths([ get_option('prefix'), get_option('libdir') ])
mpt_inc = join_paths([ get_option('prefix'), get_option('includedir'), 'mpt' ])

# require compiler for target library search
cc = meson.get_compiler('c')

# base resources
base_dep = []
base_inc = []
base_src = []

# do not build base libraries
with_shared = false

# external core library exists
core = cc.find_library('mptcore', required : false, dirs : mpt_lib)
if core.found()
  base_inc = [ include_directories(mpt_inc) ]
  base_dep = [ core ]

  # check 
  load = cc.find_library('mptloader', required : false, dirs : mpt_lib)
  if load.found()
    base_dep += load
  else
    subdir('base/mptloader')
    base_src += load_src
  endif
  # query required plot sources
  plot = cc.find_library('mptplot', required : false, dirs : mpt_lib)
  if plot.found()
    base_dep += plot
  else
    subdir('base/mptplot/values')
    base_src += src
    base_inc += include_directories('base/mptplot')
  endif
else
  subdir('base/mptcore')
  base_src += core_src
  base_inc += core_inc
  subdir('base/mptloader')
  base_src += load_src
  base_dep += load_dep
  subdir('base/mptplot/values')
  base_src += src
  base_inc += include_directories('base/mptplot')
endif


# optional library for client
with_io = get_option('with_io')
if with_io
  io = cc.find_library('mptio', required : false, dirs : mpt_lib)
  if not io.found()
    subdir('base/mptio')
    base_src += io_src
    base_inc += io_inc
  endif
endif

# need to build private sources
if base_src.length() != 0
  base = static_library('mpt', base_src,
    include_directories : base_inc,
    pic : true)
  base = declare_dependency(link_with : base, include_directories : base_inc)
  base_dep = [ base ] + base_dep
endif

install_headers('solver.h', subdir : 'mpt')

# solver library creation
solv_inc = include_directories('.', 'base')

# get library options for current project
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
subdir('mptsolver')

# solver modules creation
solv_mod_inc = [ solv_inc, base_inc, include_directories('modules') ]
subdir('modules/sundials')

subdir('examples')
