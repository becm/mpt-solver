# MPT solver interface

Create and operate on numeric solvers implementing MPT
client interface.

## Solver utilities
Operations on solver client configuration and data.

Additional helpers for solver output and execution.

## Client creation
A generic client can be created for a specific problem type.

This client supports dynamic loading of suitable solver modules
whose init symbol or shortname are taken from the client configuration
element `mpt.client.solver`.

## Solver modules
Each module library supports creation of instances conforming
to MPT generic solver interface.

To create solver modules additional `math` sources/objects
are required, infos on how to obtain these files, special build instructions
and further info can be found in `modules/<name>/<name>.info`.

The generated modules are subject to licences from this project and
the ones covering their `math` sources!
Make shure to understand all license implications before redistribution
in any form.
