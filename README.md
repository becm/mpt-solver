# MPT solver extensions
This is a collection of bindings to various numeric solvers using consistent
calling conventions to allow code reuse of user problem formulations.

Solvers can be set up using simple text configuration files.
Wrapper instances conform to interfaces and use resource management compatible
with basic [MPT](https://github.com/becm/mpt-base) types.

Please have a look at the [examples](./examples) folder to see some simple use
cases and also full user code for well-known numeric problems utilizing the
whole stack of supplied routines.


## Solver utility library
An optional runtime is provided for easy adjustment of problem and solver setups
by applying configuration values from user-readable structured text files:
- solver binding and settings to be used
- initial values or profile for 1D-PDE distribution
- data export filters

User processes are controlled via command line or remote connection
using `mpt::client` as a generic interface abstraction.


### Client implementations
The suppliend implementations for nonlinear and IVP problems use
dynamic loading of suitable solver modules and use differently structured
configuration files for problem and solver settings by default.

Config file content is made available at `"mpt.client"` in the config tree.
See documentation of `mpt_init()` for additional command line handling.


## Solver modules
Each module library supports creation of metatype references with interfaces
for generic MPT object and solver operations.

Named aliases for methods to create solver instances are defined by adding
entries to the `"mpt.loader.alias"` config hierarchy.
This can be done by placing config files in `${MPT_PREFIX}/etc/mpt.conf.d`.

Additional 3rd party files are required to build solver modules from source.
Where to obtain these files, additional requirements and further information
can be found in the respective `modules/<name>/README.md`.

The resulting solver libraries are subject to licence terms of this project
as well as their respective 3rd party `math` sources.
Make sure to understand all license implications before redistring
in any form.
