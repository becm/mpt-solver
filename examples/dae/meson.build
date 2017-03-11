# mpt-solver/examples/dae/meson.build
# DAE tests for MPT solver library

test('cakzo DAE test',
  executable('cakzo', 'cakzo.c',
    include_directories : solv_inc,
    c_args: test_defines,
    dependencies: [ runtime_ivp, solv, libm ]),
  args: files('cakzo.conf'))

test('robertson DAE test',
  executable('robertson', 'robertson.c',
    include_directories : solv_inc,
    c_args: test_defines,
    dependencies: [ runtime_ivp, solv, libm ]),
  args: files('robertson.conf', 'robertson_sol.conf'))
