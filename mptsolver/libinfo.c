/*!
 * MPT client library description
 */

#include "release.h"
#include "version.h"
#include "libinfo.h"

extern void _start(void)
{
	_library_ident("MPT Solver Interface Library");
	
	_library_task("create solver clients");
	_library_task("configure solver data");
	_library_task("execute solver steps");
	
	_exit(0);
}

