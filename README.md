# MPT solver extensions
This is a collection of interfaces to various numeric solvers using a unified
configuration approach and calling conventions.

Supported solver instances use [MPT](https://github.com/becm/mpt-base)
interfaces for configuration and resource management.

The standardized data and user interaction layers for IVP and nonlinear solvers
are designed to support high reusability and reasonable accessibility for
a broad user base.


## Solver utility library
The solver runtime uses specific MPT data types
or their generic representation for most interactions.

Solver implementaions are taken from runtime-loadable shared libraries.

Setting up solvers for specific problems is done via configuration files.


### Client implementations
User processes to solve nonlinear and IVP problems can be controlled via
command line or remote connection using `mpt::client` as generic
interface abstraction.

Both suppliend client implementations use dynamic loading of
suitable solver modules and read problem and solver
configuration from files.

The client config root node is placed into the global MPT configuration.
By default, the location `"mpt.client"` is used as the main problem setup file.
See documentation of `mpt_init()` for additional command line handling.


## Solver modules
Each module library supports creation of metatype references with interfaces
for generic MPT object and solver operations.

To build solver modules, additional `math` sources/objects
are required. Where to obtain 3rd party files, library build instructions
and further information can be found in `modules/<name>/README.md`.

Files installed to `${MPT_PREFIX}/etc/mpt.conf.d` are used to map named aliases
to solver instance builder methods.

The generated solver libraries are subject to licences from this project and
the ones covering their `math` sources!
Make shure to understand all license implications before redistribution
in any form.
