/*!
 * MPT solver module helper function
 *   assign next time step target
 */

#include "../solver.h"

#include "meta.h"

extern int mpt_solver_module_nextval(double *next, double min, MPT_INTERFACE(convertable) *src)
{
	double end = *next;
	int ret;
	if (!src) {
		if (end < min) {
			return MPT_ERROR(BadValue);
		}
		return 0;
	}
	if ((ret = src->_vptr->convert(src, 'd', &end)) < 0) {
		return ret;
	}
	if (end < min) {
		return MPT_ERROR(BadValue);
	}
	*next = end;
	return 0;
}
