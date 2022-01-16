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
	if (tol->_base) {
		if (val) {
			val->type = MPT_type_toVector('d');
			if (val->_bufsize <= sizeof(*tol)) {
				val->ptr = memcpy(val->_buf, tol, sizeof(*tol));
				return 2;
			}
			val->ptr = tol;
		}
		return 0;
	}
	if (val) {
		val->type = 'd';
		if (val->_bufsize <= sizeof(tol->_d.val)) {
			val->ptr = memcpy(val->_buf, &tol->_d.val, sizeof(tol->_d.val));
			return 1;
		}
		val->ptr = &tol->_d.val;
	}
	return 0;
}
