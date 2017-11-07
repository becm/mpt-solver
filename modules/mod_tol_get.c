/*!
 * MPT solver module helper function
 *   prepare vecpar data
 */

#include <string.h>
#include <stdlib.h>
#include <sys/uio.h>

#include "meta.h"

#include "../solver.h"

extern int mpt_solver_module_tol_get(const MPT_SOLVER_TYPE(dvecpar) *tol, MPT_STRUCT(value) *val)
{
	int len = tol->base ? tol->d.len/sizeof(double) : 0;
	
	if (!val) {
		return len;
	}
	if (tol->base) {
		static const char fmt[2] = { MPT_value_toVector('d') };
		val->fmt = fmt;
		val->ptr = tol;
	} else {
		static const char fmt[2] = "d";
		val->fmt = fmt;
		val->ptr = &tol->d.val;
	}
	return len;
}
