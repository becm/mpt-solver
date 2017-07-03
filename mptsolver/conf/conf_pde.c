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
extern int mpt_conf_pde(MPT_STRUCT(solver_data) *md, const MPT_STRUCT(node) *conf, MPT_INTERFACE(logger) *out)
{
	int len;
	
	/* read user parameter to parameter matrix */
	if (!md->npar) {
		static const char pname[] = "param";
		if ((len = mpt_conf_param(&md->param, mpt_node_next(conf, pname), 0)) < 0) {
			if (out) mpt_log(out, __func__, MPT_LOG(Warning), "%s: %s",
			                 pname, MPT_tr("invalid parameter format"));
		} else {
			md->npar = len;
		}
	}
	/* grid setup */
	if (!md->nval) {
		const MPT_STRUCT(node) *grid = mpt_node_next(conf, "grid");
		if ((len = mpt_conf_grid(&md->val, grid ? grid->_meta : 0)) < 0) {
			if (out) mpt_log(out, __func__, MPT_LOG(Error), "%s",
			                 MPT_tr("unable to get grid data"));
			return MPT_ERROR(BadValue);
		}
		md->nval = len;
		
		if (--len < 1) {
			if (out) mpt_log(out, __func__, MPT_LOG(Error), "%s: %d",
			                 MPT_tr("bad grid interval count"), len);
			return MPT_ERROR(BadValue);
		}
		return len;
	}
	return 0;
}

