/*!
 * set Sundials jacobian parameters
 */

#include <stdio.h>
#include <sundials/sundials_direct.h>

#include "sundials.h"

#include "meta.h"

/*!
 * \ingroup mptSundials
 * \brief set SUNDIALS jacobian
 * 
 * Set parameters for SUNDIALS jacobian.
 * 
 * \return number of consumed values
 */
extern int mpt_sundials_jacobian(MPT_SOLVER_STRUCT(sundials) *sd, long neqs, const MPT_INTERFACE(metatype) *src)
{
	MPT_STRUCT(consumable) val;
	const char *key;
	int ret;
	char mode;
	
	if (!src) {
		sd->stype = 0;
		return 0;
	}
	key = 0;
	if ((ret = mpt_consumable_setup(&val, src)) < 0) {
		if ((ret = src->_vptr->conv(src, 'k', &key)) < 0) {
			return ret;
		}
		ret = 0;
	}
	else if ((ret = mpt_consume_key(&val, &key)) < 0) {
		return ret;
	} else {
		ret = 1;
	}
	if (!key || !(mode = *key)) {
		sd->stype = 0;
		return ret;
	}
	switch (mode) {
		case 'd': case 'D':
			sd->jacobian = 0;
			sd->stype = MPT_SOLVER_SUNDIALS(Direct);
			return ret;
		case 'f':
			sd->jacobian = SUNDIALS_DENSE;
			sd->stype = MPT_SOLVER_SUNDIALS(Direct) | MPT_SOLVER_SUNDIALS(Numeric);
			return ret;
		case 'F':
			sd->jacobian = SUNDIALS_DENSE;
			sd->stype = MPT_SOLVER_SUNDIALS(Direct);
			return ret;
		case 'b': case 'B':
			sd->jacobian = SUNDIALS_BAND;
			sd->stype = MPT_SOLVER_SUNDIALS(Direct);
			
			if ((ret = mpt_consume_int(&val, &sd->ml)) <= 0) {
				sd->ml = sd->mu = neqs;
			}
			else if ((ret = mpt_consume_int(&val, &sd->mu)) <= 0) {
				sd->mu = sd->ml;
				ret = 2;
			}
			if (mode != 'B') {
				sd->stype |= MPT_SOLVER_SUNDIALS(Numeric);
			}
			return ret;
		default:
			return MPT_ERROR(BadValue);
	}
}
