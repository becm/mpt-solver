/*!
 * BACOL library description
 */

#include "release.h"
#include "libinfo.h"

extern void _start()
{
	_library_ident("BACOL solver library");
	
	_library_task("interface to generic solver");
	_library_task("operations on BACOL data");
#ifdef MPT_BACOL_DASSL
	_library_task("BACOL solver with 'dassl' backend");
#endif
#ifdef MPT_BACOL_RADAU
	_library_task("BACOL solver with 'radau' backend");
#endif
	_exit(0);
}
