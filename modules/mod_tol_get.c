/*!
 * MPT solver module helper function
 *   prepare vecpar data
 */

#include <string.h>

#include "../solver.h"

extern int mpt_solver_module_tol_get(MPT_STRUCT(property) *pr, const MPT_SOLVER_TYPE(dvecpar) *tol)
{
	if (tol->_base) {
		if (pr) {
			MPT_value_set(&pr->val, MPT_type_toVector('d'), tol);
			if (tol && (sizeof(pr->_buf) <= sizeof(*tol))) {
				pr->val.ptr = memcpy(pr->_buf, tol, sizeof(*tol));
				return 2;
			}
		}
		return 0;
	}
	if (pr) {
		MPT_value_set(&pr->val, 'd', &tol->_d.val);
		if (tol && (sizeof(pr->_buf) <= sizeof(tol->_d.val))) {
			pr->val.ptr = memcpy(pr->_buf, &tol->_d.val, sizeof(tol->_d.val));
			return 1;
		}
	}
	return 0;
}
