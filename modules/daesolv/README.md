# DAE solver module
Interfaces to
[ddassl](https://www.netlib.org/ode/ddassl.f),
[mebdfi](https://archimede.dm.uniba.it/~testset/solvers/mebdfi.php) and
[radau](https://archimede.dm.uniba.it/~testset/solvers/radau.php)
solvers.

A successor to `dassl` called `IDA` is used in the [SUNDIALS](../sundials) module.

Auxiliary routines can be obtained from [LINPACK](https://netlib.org/linpack/).

# Local Changes to `mebdfi.f`
- fix initialization of EPSJAC (not saved)
- drop INTERP() call in 8000 block (no access to actual Y0)
- remove DEC() and SOL() -> use versions in `radaua.f`
- remove bundled LINPACK and BLAS routines
