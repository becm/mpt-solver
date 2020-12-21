/*!
 * DASSL solver library description
 */

#include "libinfo.h"

extern void _start()
{
	_library_ident("DAE solver library");
	
	_library_task("interfaces to generic IVP solver");
	_library_task("operations on data for included solvers");
	_library_task("DASSL solver backend");
	_library_task("RADAU solver backend");
	_library_task("MEBDFI solver backend");
	
	_exit(0);
}

