/*!
 * Sundials interface library description
 */

#include <sundials/sundials_config.h>

#include "libinfo.h"

extern void _start()
{
	_library_ident("Sundials '"SUNDIALS_VERSION"' solver library");
	
	_library_task("generic interface/operations for included solvers");
	_library_task("IDA differential-algebraic equotation solver");
	_library_task("CVODE differential equotation solver");
	_library_task("KINSOL nonlinear equotation solver");
	
	_exit(0);
}
