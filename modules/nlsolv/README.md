# Nonlinear solver wrappers
Interface to standalone solvers for nonlinear problems (normal and overdetermined).

The [MINPACK](https://netlib.org/minpack/) suite can be used to solve normal and
overdetermined problems.

Multiple variants of the N2 solver faily of the [PORT](https://netlib.org/port/)
suite are supported to cover different options of problem formulations.

See selection of [MINPACK](minpack/minpack.mk) and [N2](portn2/portdn2.mk)
source files for 3rd party requirements.
