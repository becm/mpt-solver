# mpt-solver/examples/meson.build
# tests for MPT solver library

test_defines = [ '-DMPT_INCLUDE(x)=#x' ]

if with_cxx
  test('vector parameter output',
    executable('vecpar', 'vecpar.cpp',
      include_directories : solv_inc,
      cpp_args: test_defines,
      dependencies: solv))

  if is_variable('mod_sundials')
    test('solver status output',
      executable('solvers', 'solvers.cpp',
        include_directories : solv_inc,
        cpp_args: test_defines + [ '-Dwith_sundials' ],
        dependencies: [ solv, cxx, mod_sundials ]))
  endif
endif

