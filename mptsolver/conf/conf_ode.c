/*!
 * create IVP solver parameter from configuration list.
 */

#include <math.h>

#include "array.h"
#include "node.h"

#include "values.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief configure ODE data
 * 
 * Get data for ODE solver form configuration.
 * 
 * \param md   client data descriptor
 * \param conf client configuration list
 * \param out  error log descriptor
 * 
 * \return equotation count
 */
extern int mpt_conf_ode(MPT_STRUCT(solver_data) *md, double t, const MPT_STRUCT(node) *conf, MPT_INTERFACE(logger) *out)
{
	MPT_STRUCT(buffer) *buf;
	MPT_STRUCT(node) *prof;
	int len;
	
	if (!md->npar) {
		static const char pname[] = "param";
		if ((len = mpt_conf_param(&md->param, mpt_node_next(conf, pname), 0)) < 0) {
			if (out) mpt_log(out, __func__, MPT_LOG(Error), "%s: %s",
			                 pname, MPT_tr("invalid parameter format"));
			return MPT_ERROR(BadValue);
		} else {
			md->npar = len;
		}
	}
	if (md->nval) {
		return 0;
	}
	/* save initial time value */
	if ((buf = md->val._buf)) buf->_used = 0;
	if (!mpt_array_append(&md->val, sizeof(t), &t)) {
		if (out) mpt_log(out, __func__, MPT_LOG(Error), "%s",
		                 MPT_tr("unable to save initial time in history"));
		return MPT_ERROR(BadOperation);
	}
	if (!(prof = mpt_node_next(conf, "profile"))) {
		len = 0;
	}
	/* read profile values */
	else if ((len = mpt_conf_param(&md->val, prof, 1)) < 0) {
		if (out) mpt_log(out, __func__, MPT_LOG(Error), "%s",
		                 MPT_tr("failed to reserve initial data"));
		return len;
	}
	md->nval = len + 1;
	
	return len;
}

