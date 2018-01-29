# mpt-solver/examples/pde/meson.build
# PDE tests for MPT solver library

test('diffusion PDE test',
  executable('diff', 'diff.c',
    c_args: test_defines,
    dependencies: [ runtime_ivp ]),
  args: [
    files('diff.conf'),
    'solconf=' + join_paths(meson.current_source_dir(), 'diff_sol.conf')
  ],
  env : [ 'LD_LIBRARY_PATH=' + examples_ld ]
)

test('flame PDE test',
  executable('flame', 'flame.c',
    c_args: test_defines,
    dependencies: [ runtime_ivp, libm ]),
  args: [
    files('flame.conf'),
    'solconf=' + join_paths(meson.current_source_dir(), 'flame_sol.conf')
  ],
  env : [ 'LD_LIBRARY_PATH=' + examples_ld ]
)

test('medical akzo PDE test',
  executable('makzo', 'makzo.c',
    c_args: test_defines,
    dependencies: [ runtime_ivp ]),
  args: files('makzo.conf'),
  env : [ 'LD_LIBRARY_PATH=' + examples_ld ]
)

test('two equotation PDE test',
  executable('twoeq', 'twoeq.c',
    c_args: test_defines,
    dependencies: [ runtime_ivp, libm ]),
  args: [
    files('twoeq.conf'),
    'solconf=' + join_paths(meson.current_source_dir(), 'twoeq_sol.conf')
  ],
  env : [ 'LD_LIBRARY_PATH=' + examples_ld ]
)
