/*!
 * create PDE solver parameter from configuration list.
 */

#include <math.h>

#include "node.h"
#include "array.h"

#include "values.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief configure PDE data
 * 
 * Get data for PDE solver form configuration.
 * 
 * \param md   client data descriptor
 * \param conf client configuration list
 * \param out  error log descriptor
 * 
 * \return PDE count
 */
extern int mpt_conf_pde(MPT_SOLVER_STRUCT(data) *md, const MPT_STRUCT(node) *conf, MPT_INTERFACE(logger) *out)
{
	const char *val;
	int	len;
	
	if (!md->iter && mpt_conf_iterator(&md->iter, mpt_node_next(conf, val = "times")) < 0) {
		(void) mpt_log(out, __func__, MPT_ENUM(LogError), "%s: %s", val, MPT_tr("invalid iterator description"));
		return -1;
	}
	/* read user parameter to parameter matrix */
	if (!md->npar) {
		if ((len = mpt_conf_param(&md->param, mpt_node_next(conf, val = "param"), 0)) < 0) {
			(void) mpt_log(out, __func__, MPT_ENUM(LogError), "%s: %s", val, MPT_tr("invalid parameter format"));
		} else {
			md->npar = len;
		}
	}
	/* grid setup */
	if (!md->nval) {
		if ((len = mpt_conf_grid(&md->val, mpt_node_next(conf, "grid"))) < 0) {
			(void) mpt_log(out, __func__, MPT_ENUM(LogError), "%s", MPT_tr("unable to get grid data"));
			return -2;
		}
		return 1;
	}
	return 0;
}

