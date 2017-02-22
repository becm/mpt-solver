# mpt-solver/examples/meson.build
# tests for MPT solver library

test_defines = [ '-DMPT_INCLUDE(x)=#x' ]

if get_option('with_cxx')
  add_languages('cpp')
  
  test('vector parameter output',
    executable('vecpar', 'vecpar.cpp',
      cpp_args: test_defines,
      include_directories : base_inc + [ solv_inc ],
      dependencies: solv))
endif
