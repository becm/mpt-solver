# mpt-solver/examples/ode/meson.build
# ODE test for MPT solver library

test('crackoil ODE test',
  executable('crackoil', 'crackoil.c',
    c_args: test_defines,
    dependencies: [ runtime_ivp ]),
  args: files('crackoil.conf'),
  env : [ 'LD_LIBRARY_PATH=' + examples_ld ]
)
