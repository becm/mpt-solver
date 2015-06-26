/*!
 * set double values on vector
 */

#include <stdlib.h>

#include "solver.h"

extern int mpt_ivppar_set(MPT_SOLVER_STRUCT(ivppar) *ivp, MPT_INTERFACE(source) *src)
{
	double t;
	int32_t l1, l2;
	
	if (!src) {
		l1 = 0;
		if (ivp->neqs < 0) return MPT_ENUM(BadArgument);
		if (ivp->pint < 0) return MPT_ENUM(BadValue);
		if (ivp->neqs != 1)   l1 |= 1;
		if (ivp->pint != 0)   l1 |= 2;
		if (ivp->last != 0.0) l1 |= 4;
		return l1;
	}
	if ((l1 = src->_vptr->conv(src, 'i', &l2)) > 0) {
		int32_t pi;
		if (l2 == 0) {
			return MPT_ENUM(BadValue);
		}
		if (l2 > 0) {
			ivp->neqs = l2;
		}
		if ((l2 = src->_vptr->conv(src, 'i', &pi)) == 0) {
			ivp->pint = 0;
			l2 = 0;
		} else if (l2 > 0 && pi >= 0) {
			ivp->pint = pi;
		}
		l1 += l2;
	} else {
		l1 = 0;
	}
	t = 0;
	if ((l2 = src->_vptr->conv(src, 'd', &t)) > 0) {
		ivp->last = t;
		l1 += l2;
	}
	
	return l1;
}
