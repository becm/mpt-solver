# mpt-solver/examples/dae/meson.build
# DAE tests for MPT solver library

test('cakzo DAE test',
  executable('cakzo', 'cakzo.c',
    c_args: test_defines,
    dependencies: [ runtime_ivp, libm ]),
  args: files('cakzo.conf'),
  env : [ 'LD_LIBRARY_PATH=' + examples_ld ]
)

test('robertson DAE test',
  executable('robertson', 'robertson.c',
    c_args: test_defines,
    dependencies: [ runtime_ivp ]),
  args: [
    files('robertson.conf'),
    'solconf=' + join_paths(meson.current_source_dir(), 'robertson_sol.conf')
  ],
  env : [ 'LD_LIBRARY_PATH=' + examples_ld ]
)
