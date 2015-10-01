# mpt-solver/meson.build
# build definitions for mpt solvers

project('mpt', 'c')

if not meson.is_subproject()
  add_global_arguments('-std=c++11', language : 'cpp')
endif

inc = include_directories(meson.current_source_dir())
deps = [
  dependency('mptio', required : false),
  dependency('mptplot', required : false),
  dependency('mptcore', required : false)
]

has_mpt = true
foreach d : deps
  if not d.found()
    has_mpt = false
  endif
endforeach

if not has_mpt
  mpt  = subproject('mpt-base')
  deps = declare_dependency(
    include_directories : mpt.get_variable('inc'),
    link_with : [
      mpt.get_variable('mptplot'),
      mpt.get_variable('mptio'),
      mpt.get_variable('mptcore')
    ]
  )
else
endif

install_headers('solver.h', install_dir : 'include/mpt')

subdir('mptsolver')
