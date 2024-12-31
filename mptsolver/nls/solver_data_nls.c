/*!
 * MPT solver library
 *   save nonlinear system parameters
 */

#include <string.h>

#include <sys/uio.h>

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief save nonlinear system parameters
 * 
 * Save parameter state to solver data parameters.
 * 
 * \param dat  solver data
 * \param val  current nonlinear solver values
 * 
 * \return number of changed elements
 */
extern int mpt_solver_data_nls(MPT_STRUCT(solver_data) *dat, const MPT_STRUCT(value) *val)
{
	const struct iovec *vec;
	const double *par;
	double *dst;
	int np;
	
	if (!val || !dat) {
		return MPT_ERROR(BadArgument);
	}
	if (val->_type == MPT_type_toVector('d')) {
		if (!(vec = val->_addr)) {
			return MPT_ERROR(BadValue);
		}
	}
	else if (val->_type == MPT_ENUM(TypeObjectPtr)) {
		MPT_STRUCT(property) pr;
		const MPT_INTERFACE(object) *obj = *((void * const *) val->_addr);
		
		pr.name = "parameters";
		if (obj->_vptr->property(obj, &pr) < 0
		 || !(vec = pr.val._addr)) {
			return MPT_ERROR(MissingData);
		}
		if (pr.val._type != MPT_type_toVector('d')) {
			return MPT_ERROR(BadType);
		}
	}
	else {
		return MPT_ERROR(BadType);
	}
	if (!(par = vec->iov_base)
	 || !(np = vec->iov_len / sizeof (*par))) {
		return 0;
	}
	/* copy output so state data */
	if ((dst = mpt_solver_data_param(dat))) {
		int len;
		if ((len = dat->npar) > np) {
			len = np;
		}
		memcpy(dst, par, len * sizeof(*par));
	}
	return np;
}
