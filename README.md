# MPT solver extensions

Configure numeric solvers via MPT object operations.

Further abstractions allow unification of user implementations
for various IVP and nonlinear solver types.


## Solver utility library
Operations for solvers conforming to MPT interfaces.

Create solver instances from runtime-loadable shared
libraries with aliases supplied by solver modules.

Set up solvers for specific problems with
data read from configuration files.

### Client implementations
Control solver processes for nonlinear and IVP problems via
command line or remote connection using `mpt::client` as
interface abstraction.

Both suppliend client implementations use dynamic loading of
suitable solver modules and read problem and solver
configuration from files.

The client config root node is placed into the global MPT configuration.
Use `"mpt.client"` as config location to accept the default filename
set by `mpt_init()` from command line arguments.


## Solver modules
Each module library supports creation of instances conforming
to generic MPT solver interface.

To create solver modules additional `math` sources/objects
are required, infos on how to obtain these files, special build instructions
and further info can be found in `modules/<name>/<name>.info`.

Solvers module aliases installed to `${MPT_PREFIX}/etc/mpt.conf.d`
are used by standard resolution methods.

The generated modules are subject to licences from this project and
the ones covering their `math` sources!
Make shure to understand all license implications before redistribution
in any form.
