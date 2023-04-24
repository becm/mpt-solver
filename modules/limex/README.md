# LIMEX solver module
Interface to [LIMEX](http://www.zib.de/weimann/CodeLib/en/ivpode.html) DAE solver.

The solver implementation uses a fixed size glabal work array.
Only one wrapper instance is allowed to exist at any point
during process runtime.
