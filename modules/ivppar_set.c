/*!
 * set double values on vector
 */

#include <stdlib.h>

#include "solver.h"

extern int mpt_ivppar_set(MPT_SOLVER_STRUCT(ivppar) *ivp, MPT_INTERFACE(metatype) *src)
{
	double t;
	int32_t l1, l2;
	
	if (!src) {
		ivp->neqs = 1;
		ivp->pint = 0;
		ivp->last = 0;
		return 0;
	}
	if ((l1 = src->_vptr->conv(src, 'i' | MPT_ENUM(ValueConsume), &l2)) > 0) {
		int32_t pi;
		if (l2 == 0) {
			return MPT_ERROR(BadValue);
		}
		if (l2 > 0) {
			ivp->neqs = l2;
		}
		if ((l2 = src->_vptr->conv(src, 'i' | MPT_ENUM(ValueConsume), &pi)) == 0) {
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
	if ((l2 = src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), &t)) > 0) {
		ivp->last = t;
		l1 += l2;
	}
	
	return l1;
}
