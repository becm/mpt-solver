/*!
 * MPT solver module helper function
 *   prepare vecpar data
 */

#include <string.h>
#include <stdlib.h>
#include <sys/uio.h>

#include "types.h"

#include "../solver.h"

extern int mpt_solver_module_tol_get(MPT_STRUCT(value) *val, const MPT_SOLVER_TYPE(dvecpar) *tol)
{
	int len;
	
	if (tol->_base) {
		len = tol->_d.len / sizeof(double);
		if (val) {
			static const uint8_t fmt[2] = { MPT_type_toVector('d') };
			val->fmt = fmt;
			val->ptr = tol->_base;
		}
	} else {
		len = 0;
		if (val) {
			static const uint8_t fmt[2] = "d";
			val->fmt = fmt;
			val->ptr = &tol->_d.val;
		}
	}
	return len;
}
