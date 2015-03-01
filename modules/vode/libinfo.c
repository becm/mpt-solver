/*!
 * VODE solver library description
 */

#include "release.h"
#include "libinfo.h"

extern void _start()
{
	_library_ident("VODE solver library");
	
	_library_task("interface to generic IVP solver");
	_library_task("operations on VODE data");
	_library_task("VODE solver backend");
	
	_exit(0);
}
