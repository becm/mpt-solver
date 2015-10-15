/*!
 * library description standalone nonlinear solvers
 */

#include "version.h"
#include "libinfo.h"

extern void _start()
{
	_library_ident("solver library for nonlinear systems");
	
	_library_task("interfaces to generic nonlinear solver");
	_library_task("operations on data for included solvers");
	_library_task("Levenberg-Marquart from MINPACK");
	_library_task("Powell Hybrid from MINPACK");
	_library_task("N2 for double precission from PORT");
	
	_exit(0);
}
