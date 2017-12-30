# mpt-solver/examples/meson.build
# tests for MPT solver library

test_defines = [ '-DMPT_INCLUDE(x)=#x' ]

examples_ld = '$ORIGIN'
foreach d : solv_mod_lib
  examples_ld += ':' + d
endforeach

if with_cxx
  test('vector parameter output',
    executable('vecpar', 'vecpar.cpp',
      include_directories : solv_inc,
      cpp_args: test_defines,
      dependencies: cxx)
  )
  if get_option('with_io')
    test('client instance',
      executable('client', 'client.cpp', 'solver_run.c',
        include_directories : solv_inc,
        c_args: test_defines,
        cpp_args: test_defines,
        dependencies: [ cxx, solv, io ])
    )
  endif
  
  if is_variable('mod_sundials')
    test('solver status output',
      executable('solvers', 'solvers.cpp',
        include_directories : solv_inc,
        cpp_args: test_defines + [ '-Dwith_sundials' ],
        dependencies: [ solv, cxx, mod_sundials ]),
      env : [ 'LD_LIBRARY_PATH=' + examples_ld ]
    )
  endif
endif

if get_option('with_io') and is_variable('mod_sundials')
  runtime_ivp = declare_dependency(
    dependencies : [ solv, io ],
    include_directories : include_directories('.'),
    sources : files('solver_run.c'))
  subdir('dae')
  subdir('ode')
  subdir('pde')
endif
