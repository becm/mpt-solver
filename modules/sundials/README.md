# SUNDIALS solver module
Interface to solvers in the [SUNDIALS](https://computing.llnl.gov/projects/sundials)
library.

## Local Build
To use *SUNDIALS* libraries with `Makefile` build,
set `SUNDIALS` variable to *SUNDIALS* source directory
and `SUNDIALS_BUILD` to *SUNDIALS* CMake build directory.

The default config expects a (versioned) subdirectory
in the general `MATH_BASE` location.

### Alternative `Makfile` build modes
Shared  versions of *SUNDIALS* libraries can be used by removing `MATH_OBJS_{SHARED,STATIC}`
definitions from and add *SUNDIALS* libraries to linker arguments in `Makefile`.

To disable *LAPACK* support in wrapper,
comment out `sundials_lapack` definition.

### Special vector types
The default implementation uses serial *SUNDIALS* `N_Vector` instances.
A different vector backend can be set by implementing `mpt_sundials_nvector()`
in user program and link in before `libmpt_sundials.so`.
