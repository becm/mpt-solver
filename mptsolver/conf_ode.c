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
extern int mpt_conf_ode(MPT_SOLVER_STRUCT(data) *md, const MPT_STRUCT(node) *conf, MPT_INTERFACE(logger) *out)
{
	MPT_STRUCT(buffer) *buf;
	MPT_STRUCT(node) *prof;
	const char *val;
	double t;
	int len;
	
	/* check iterator */
	if (!md->iter && mpt_conf_iterator(&md->iter, mpt_node_next(conf, val = "times")) < 0) {
		(void) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s: %s", val,
		               MPT_tr("invalid iterator description"));
		return -1;
	}
	
	if (!md->npar) {
		if ((len = mpt_conf_param(&md->param, mpt_node_next(conf, val = "param"), 0)) < 0) {
			(void) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s: %s",
			               val, MPT_tr("invalid parameter format"));
		} else {
			md->npar = len;
		}
	}
	if (md->nval) return 0;
	
	t = mpt_iterator_curr(md->iter);
	if (isnan(t)) {
		(void) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s", val,
		               MPT_tr("unable to get iterator state"));
		return -1;
	}
	/* save initial independant value */
	if ((buf = md->val._buf)) buf->used = 0;
	if (!mpt_array_append(&md->val, sizeof(t), &t)) {
		(void) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s",
		               MPT_tr("unable to save initial time in history"));
		return -4;
	}
	if (!(prof = mpt_node_next(conf, "profile"))) {
		len = 0;
	}
	/* read profile values */
	else if ((len = mpt_conf_param(&md->val, prof, 1)) < 0) {
		(void) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s: %s",
		               val, MPT_tr("failed to reserve initial data"));
		return -1;
	}
	buf = md->val._buf;
	md->nval = buf->used/sizeof(double);
	
	return len;
}

