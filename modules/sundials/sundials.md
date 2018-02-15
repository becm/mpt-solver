# SUNDIALS module for [MPT](https://github.com/becm/mpt-solver)

## Source Information
Use solvers from [SUNDIALS](https://computation.llnl.gov/projects/sundials)
library

## Local Build
To use *SUNDIALS* libraries with `Makefile` build
set `DIR_SUNDIALS_OBJ` to *SUNDIALS* CMake build directory
and `DIR_SUNDIALS_INC` to *SUNDIALS* install include directory.

### Alternative `Makfile` build modes
To use shared *SUNDIALS* libraries remove `MATH_OBJS_{SHARED,STATIC}`
definitions and add *SUNDIALS* libraries to linker arguments.

To disable *LAPACK* support in wrapper comment out `sundials_lapack`
definition.

### Special vector types
Library creates serial *SUNDIALS* `N_Vector` instances by default.
To use different vector backend, reimplement `mpt_sundials_nvector()`
in user program and link in before `libmpt_sundials.so`.