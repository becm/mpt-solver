/*!
 * set Sundials jacobian parameters
 */

#include "meta.h"

#include "sundials.h"

extern int sundials_jacobian(MPT_SOLVER_STRUCT(sundials) *sd, int neqs, MPT_INTERFACE(metatype) *src)
{
	char *key;
	int l1, l2, l3;
	
	if (!src) return 0;
		
	if ((l1 = src->_vptr->conv(src, 'k', &key)) < 0) return l1;
	
	if (!l1 || !key) {
		sd->jacobian = MPT_SOLVER_ENUM(SundialsJacNone);
		return 0;
	}
	
	switch (key[0]) {
		case 'd': case 'D':
			sd->jacobian = MPT_SOLVER_ENUM(SundialsJacDiag);
			return l1;
		case 'f':
			sd->jacobian = MPT_SOLVER_ENUM(SundialsJacDense) | MPT_SOLVER_ENUM(SundialsJacNumeric);
			return l1;
		case 'F':
			sd->jacobian = MPT_SOLVER_ENUM(SundialsJacDense);
			return l1;
		case 'b': case 'B':
			sd->jacobian = MPT_SOLVER_ENUM(SundialsJacBand);
			
			if ((l2 = src->_vptr->conv(src, 'i', &sd->ml)) <= 0) {
				l2 = l3 = 0;
				sd->ml = sd->mu = neqs;
			}
			else if ((l3 = src->_vptr->conv(src, 'i', &sd->mu)) <= 0) {
				l3 = 0;
				sd->mu = sd->ml;
			}
			if (key[0] != 'B') sd->jacobian |= MPT_SOLVER_ENUM(SundialsJacNumeric);
			
			return l1 + l2 + l3;
		default:
			return MPT_ERROR(BadValue);
	}
}
