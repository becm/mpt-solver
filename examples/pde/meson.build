# mpt-solver/examples/pde/meson.build
# PDE tests for MPT solver library

test('diffusion PDE test',
  executable('diff', 'diff.c',
    include_directories : solv_inc,
    c_args: test_defines,
    dependencies: [ runtime_ivp, solv, libm ]),
  args: files('diff.conf', 'diff_sol.conf'))

test('flame PDE test',
  executable('flame', 'flame.c',
    include_directories : solv_inc,
    c_args: test_defines,
    dependencies: [ runtime_ivp, solv, libm ]),
  args: files('flame.conf', 'flame_sol.conf'))

test('medical akzo PDE test',
  executable('makzo', 'makzo.c',
    include_directories : solv_inc,
    c_args: test_defines,
    dependencies: [ runtime_ivp, solv, libm ]),
  args: files('makzo.conf'))

test('two equotation PDE test',
  executable('twoeq', 'twoeq.c',
    include_directories : solv_inc,
    c_args: test_defines,
    dependencies: [ runtime_ivp, solv, libm ]),
  args: files('twoeq.conf', 'twoeq_sol.conf'))
