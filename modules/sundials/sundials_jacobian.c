/*!
 * set Sundials jacobian parameters
 */

#include "module.h"

#include "sundials.h"

/*!
 * \ingroup mptSundials
 * \brief set SUNDIALS jacobian
 * 
 * Set parameters for SUNDIALS jacobian.
 * 
 * \return number of consumed values
 */
extern int sundials_jacobian(MPT_SOLVER_STRUCT(sundials) *sd, long neqs, const MPT_INTERFACE(metatype) *src)
{
	MPT_STRUCT(module_value) val = MPT_MODULE_VALUE_INIT;
	const char *key;
	int ret;
	char mode;
	
	if (!src) {
		sd->jacobian = MPT_SOLVER_ENUM(SundialsJacNone);
		return 0;
	}
	key = 0;
	if ((ret = mpt_module_value_init(&val, src)) < 0) {
		if ((ret = src->_vptr->conv(src, 'k', &key)) < 0) {
			return ret;
		}
		ret = 0;
	}
	else if ((ret = mpt_module_value_key(&val, &key)) < 0) {
		return ret;
	} else {
		ret = 1;
	}
	if (!key || !(mode = *key)) {
		sd->jacobian = MPT_SOLVER_ENUM(SundialsJacNone);
		return ret;
	}
	switch (mode) {
		case 'd': case 'D':
			sd->jacobian = MPT_SOLVER_ENUM(SundialsJacDiag);
			return ret;
		case 'f':
			sd->jacobian = MPT_SOLVER_ENUM(SundialsJacDense) | MPT_SOLVER_ENUM(SundialsJacNumeric);
			return ret;
		case 'F':
			sd->jacobian = MPT_SOLVER_ENUM(SundialsJacDense);
			return ret;
		case 'b': case 'B':
			sd->jacobian = MPT_SOLVER_ENUM(SundialsJacBand);
			
			if ((ret = mpt_module_value_int(&val, &sd->ml)) <= 0) {
				sd->ml = sd->mu = neqs;
			}
			else if ((ret = src->_vptr->conv(src, 'i', &sd->mu)) <= 0) {
				sd->mu = sd->ml;
				ret = 2;
			}
			if (mode != 'B') sd->jacobian |= MPT_SOLVER_ENUM(SundialsJacNumeric);
			
			return ret;
		default:
			return MPT_ERROR(BadValue);
	}
}
