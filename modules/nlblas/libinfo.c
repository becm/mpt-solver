/*!
 * library description for nonlinear solvers with BLAS dependancy
 */

#include "libinfo.h"

extern void _start()
{
	_library_ident("Solver for Systems of Nonlinear Equotations (with blas dependancy)");
	
	_library_task("dbconf solver from IMSL");
	_library_task("dunslf solver from IMSL");
	
	_exit(0);
}
