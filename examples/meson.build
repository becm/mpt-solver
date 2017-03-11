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

if is_variable('mod_sundials')
  runtime_ivp = declare_dependency(
    include_directories : include_directories('.'),
    sources : files('main_ivp.c', 'solver_run.c'))
  subdir('dae')
  subdir('ode')
  subdir('pde')
endif
