/*!
 * set Sundials jacobian parameters
 */

#include <errno.h>

#include "sundials.h"

extern int sundials_jacobian(MPT_SOLVER_STRUCT(sundials) *data, int neqs, MPT_INTERFACE(source) *src)
{
	char *key;
	int l1, l2, l3;
	
	if (!src) return 0;
		
	if ((l1 = src->_vptr->conv(src, 'k', &key)) < 0) return l1;
	
	if (!l1 || !key) {
		data->jacobian = MPT_ENUM(SundialsJacNone);
		return 0;
	}
	
	switch (key[0]) {
		case 'd': case 'D':
			data->jacobian = MPT_ENUM(SundialsJacDiag);
			return l1;
		case 'f':
			data->jacobian = MPT_ENUM(SundialsJacDense) | MPT_ENUM(SundialsJacNumeric);
			return l1;
		case 'F':
			data->jacobian = MPT_ENUM(SundialsJacDense);
			return l1;
		case 'b': case 'B':
			data->jacobian = MPT_ENUM(SundialsJacBand);
			
			if ((l2 = src->_vptr->conv(src, 'i', &data->ml)) <= 0) {
				l2 = l3 = 0;
				data->ml = data->mu = neqs;
			}
			else if ((l3 = src->_vptr->conv(src, 'i', &data->mu)) <= 0) {
				l3 = 0;
				data->mu = data->ml;
			}
			if (key[0] != 'B') data->jacobian |= MPT_ENUM(SundialsJacNumeric);
			
			return l1 + l2 + l3;
		default:
			errno = EINVAL;
			return MPT_ERROR(BadValue);
	}
}
