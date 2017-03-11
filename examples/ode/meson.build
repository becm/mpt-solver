# mpt-solver/examples/ode/meson.build
# ODE test for MPT solver library

test('crackoil ODE test',
  executable('crackoil', 'crackoil.c',
    include_directories : solv_inc,
    c_args: test_defines,
    dependencies: [ runtime_ivp, solv, libm ]),
  args: files('crackoil.conf'))
