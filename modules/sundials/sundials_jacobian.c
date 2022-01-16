/*!
 * set Sundials jacobian parameters
 */

#include <sundials/sundials_direct.h>

#include "types.h"
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
	MPT_INTERFACE(iterator) *it;
	const char *key;
	int32_t ml, mu;
	int ret;
	char mode;
	
	if (!src) {
		sd->linsol = 0;
		return 0;
	}
	key = 0;
	it = 0;
	if ((ret = src->_vptr->convert(src, MPT_ENUM(TypeIteratorPtr), &it)) >= 0) {
		ret = mpt_solver_module_consume_value(it, 'k', &key, 0);
	}
	if (ret < 0 && (ret = src->_vptr->convert(src, 'k', &key)) < 0) {
		return ret;
	}
	if (!key || !(mode = *key)) {
		sd->linsol = 0;
		return it ? 1 : 0;
	}
	switch (mode) {
		case 'd': case 'D':
			sd->linsol = MPT_SOLVER_SUNDIALS(Direct);
			sd->jacobian = 0;
			return it ? 1 : 0;
		case 'f': case 'F':
			sd->linsol = MPT_SOLVER_SUNDIALS(Direct);
			sd->jacobian = SUNDIALS_DENSE;
			if (mode != 'F') {
				sd->linsol |= MPT_SOLVER_SUNDIALS(Numeric);
			}
			return it ? 1 : 0;
		case 'b': case 'B':
			mu = -1;
			ml = -1;
			ret = 0;
			
			if (it) {
				int err;
				if ((err = mpt_solver_module_consume_value(it, 'i', &ml, sizeof(ml))) < 0) {
					return err;
				}
				ret = 1;
				if (!err) {
					mu = ml;
					ret = 2;
				}
				else if ((err = mpt_solver_module_consume_value(it, 'i', &mu, sizeof(mu))) < 0) {
					return MPT_ERROR(MissingData);
				}
				else {
					ret = 3;
				}
			}
			sd->ml = ml;
			sd->mu = mu;
			sd->linsol = MPT_SOLVER_SUNDIALS(Direct);
			sd->jacobian = SUNDIALS_BAND;
			if (mode != 'B') {
				sd->linsol |= MPT_SOLVER_SUNDIALS(Numeric);
			}
			return ret;
		default:
			return MPT_ERROR(BadValue);
	}
}
