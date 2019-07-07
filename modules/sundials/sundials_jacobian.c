/*!
 * set Sundials jacobian parameters
 */

#include <sundials/sundials_direct.h>

#include "meta.h"

#include "sundials.h"

/*!
 * \ingroup mptSundials
 * \brief set SUNDIALS jacobian
 * 
 * Set parameters for SUNDIALS jacobian.
 * 
 * \return number of consumed values
 */
extern int mpt_sundials_jacobian(MPT_SOLVER_STRUCT(sundials) *sd, MPT_INTERFACE(convertable) *src)
{
	MPT_STRUCT(consumable) val = MPT_CONSUMABLE_INIT;
	const char *key;
	int ret, ml, mu;
	char mode;
	
	if (!src) {
		sd->stype = 0;
		return 0;
	}
	key = 0;
	if ((ret = mpt_consumable_setup(&val, src)) < 0) {
		if ((ret = src->_vptr->convert(src, 'k', &key)) < 0) {
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
			sd->stype = MPT_SOLVER_SUNDIALS(Direct);
			sd->jacobian = 0;
			return ret;
		case 'f': case 'F':
			sd->stype = MPT_SOLVER_SUNDIALS(Direct);
			sd->jacobian = SUNDIALS_DENSE;
			if (mode != 'F') {
				sd->stype |= MPT_SOLVER_SUNDIALS(Numeric);
			}
			return ret;
		case 'b': case 'B':
			mu = -1;
			ml = -1;
			if ((ret = mpt_consume_int(&val, &ml)) <= 0) {
				ret = 0;
			}
			else if ((ret = mpt_consume_int(&val, &mu)) < 0) {
				return MPT_ERROR(BadType);
			}
			else if (!ret) {
				mu = ml;
				ret = 1;
			}
			else {
				ret = 2;
			}
			sd->ml = ml;
			sd->mu = mu;
			sd->stype = MPT_SOLVER_SUNDIALS(Direct);
			sd->jacobian = SUNDIALS_BAND;
			if (mode != 'B') {
				sd->stype |= MPT_SOLVER_SUNDIALS(Numeric);
			}
			return ret + 1;
		default:
			return MPT_ERROR(BadValue);
	}
}
