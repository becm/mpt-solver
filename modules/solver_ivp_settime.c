/*!
 * MPT solver module helper function
 *   assign next time step target
 */

#include "../solver.h"

extern int mpt_solver_ivp_settime(double *next, double min, const MPT_INTERFACE(metatype) *src)
{
	double end = *next;
	int ret;
	if (!src) {
		if (end < min) {
			return MPT_ERROR(BadValue);
		}
		return 0;
	}
	if ((ret = src->_vptr->conv(src, 'd', &end)) < 0) {
		return ret;
	}
	if (end < min) {
		return MPT_ERROR(BadValue);
	}
	*next = end;
	return 0;
}
