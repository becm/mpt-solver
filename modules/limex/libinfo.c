/*!
 * LIMEX library description
 */

#include "release.h"
#include "libinfo.h"

extern void _start()
{
	_library_ident("LIMEX solver library");
	
	_library_task("interface to generic IVP solver");
	_library_task("operations on LIMEX data");
	_library_task("LIMEX solver backend");
	
	_exit(0);
}
