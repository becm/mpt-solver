# mpt-solver/meson.build
# build definitions for mpt solver components

project('mptsolver', 'c', default_options : [
  'cpp_std=c++11',
  'c_std=c11'
])

pkg = import('pkgconfig')

mpt_lib = join_paths(get_option('prefix'), get_option('libdir'))
mpt_inc = join_paths(get_option('prefix'), get_option('includedir'), 'mpt')

# request MPT base elements
with_io = get_option('with_io')
with_cxx = get_option('with_cxx')
with_loader = true
with_plot = true

# require compiler for target library search
cc = meson.get_compiler('c')

# base resources
base_dep = []
base_src = []

# setup for base libraries
with_shared = false
with_static = true

# require core library as minimal base subset
core = cc.find_library('mptcore', required : false, dirs : mpt_lib)
if core.found()
  # external core library exists
  core_inc = include_directories(mpt_inc)
  core = declare_dependency(dependencies : core,
    link_args : '-Wl,-rpath,' + mpt_lib,
    include_directories : core_inc)

  # use existing loader library
  load = cc.find_library('mptloader', required : false, dirs : mpt_lib)
  with_loader = not load.found()
  # use existing plot library
  plot = cc.find_library('mptplot', required : false, dirs : mpt_lib)
  with_plot = not plot.found()
  # use existing I/O library or local sources
  if with_io
    io = cc.find_library('mptio', required : false, dirs : mpt_lib)
    with_io = not io.found()
  endif
  # use existing C++ base library
  if with_cxx
    cxx = cc.find_library('mpt++', required : false, dirs : mpt_lib)
    with_cxx = not io.found()
  endif
else
  # create core components locally
  core_inc = join_paths('base', 'mptcore')
  subdir(core_inc)
  core_inc = include_directories(core_inc)
  base_src += core_src
  base_dep += core
endif

# create value elements locally
if with_loader
  subdir(join_paths('base', 'mptloader'))
  base_src += load_src
  base_dep += load
endif

# create value elements locally
if with_plot
  subdir(join_paths('base', 'mptplot', 'values'))
  plot_src = src + files(
    join_paths('base', 'mptplot', 'layout_open.c'),
    join_paths('base', 'mptplot', 'history_push.c'),
    join_paths('base', 'mptplot', 'history_values.c'),
    join_paths('base', 'mptplot', 'output_local.c')
  )
  base_src += plot_src
  base_dep += declare_dependency(
      include_directories : include_directories(join_paths('base', 'mptplot')))
endif

# create I/O elements locally
if with_io
  subdir(join_paths('base', 'mptio'))
  base_src += io_src
  base_dep += io
endif

# need to build private sources
if base_src.length() != 0
  base = static_library('mpt', base_src,
    dependencies : base_dep,
    pic : true)
  if is_variable('core_src')
    core = declare_dependency(link_with : base, dependencies : base_dep)
  endif
  if is_variable('load_src')
    load = declare_dependency(link_with : base, dependencies : base_dep)
  endif
  if is_variable('plot_src')
    plot = declare_dependency(link_with : base, dependencies : base_dep)
  endif
  if is_variable('io_src')
    io = declare_dependency(link_with : base, dependencies : base_dep)
  endif
endif

# use or build C++ code
if get_option('with_cxx')
  add_languages('cpp')
  
  # need to build C++ components
  if with_cxx
    with_cxx = false  # do not install C++ library
    subdir(join_paths('base', 'mpt++'))
  endif
  with_cxx = true  # activate C++ examples
endif

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

# install header as primary project
if not meson.is_subproject()
  install_headers('solver.h', subdir : 'mpt')
endif
solv_inc = include_directories('.', 'base', join_paths('base', 'mptcore'))

# solver modules creation
libm = cc.find_library('m')
solv_mod_inc = [ include_directories('modules'), solv_inc ]
solv_mod_lib = []
subdir('modules')

# solver library creation
subdir('mptsolver')

# example creation
subdir('examples')
