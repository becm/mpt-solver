/*!
 * set double values on vector
 */

#include <stdlib.h>

#include "solver.h"

extern int mpt_ivppar_set(MPT_SOLVER_IVP_STRUCT(parameters) *ivp, MPT_INTERFACE(metatype) *src)
{
	int32_t neqs, pint;
	int len;
	
	if (!src) {
		ivp->neqs = 1;
		ivp->pint = 0;
		return 0;
	}
	if ((len = src->_vptr->conv(src, 'i' | MPT_ENUM(ValueConsume), &neqs)) < 0) {
		return len;
	}
	if (!len) {
		ivp->neqs = 1;
		ivp->pint = 0;
		return 0;
	}
	if (neqs <= 0) {
		return MPT_ERROR(BadValue);
	}
	if ((len = src->_vptr->conv(src, 'i' | MPT_ENUM(ValueConsume), &pint)) < 0) {
		return len;
	}
	if (len) {
		if (pint < 0) {
			return MPT_ERROR(BadValue);
		}
		ivp->neqs = neqs;
		ivp->pint = pint;
		return 2;
	}
	ivp->neqs = neqs;
	ivp->pint = 0;
	return 1;
}
