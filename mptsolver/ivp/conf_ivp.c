/*!
 * create PDE solver parameter from configuration list.
 */

#include <math.h>

#include "meta.h"
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
 * \param info error log descriptor
 * 
 * \return PDE count
 */
extern int mpt_conf_ivp(MPT_STRUCT(solver_data) *md, MPT_STRUCT(node) *conf, MPT_INTERFACE(iterator) *times, MPT_INTERFACE(logger) *info)
{
	MPT_STRUCT(node) *curr;
	MPT_STRUCT(buffer) *buf;
	double t;
	int len;
	
	/* create/reset config time source */
	if ((curr = mpt_node_next(conf, "times"))) {
		MPT_INTERFACE(iterator) *src;
		/* create/reset time source */
		if (!(src = mpt_conf_iter(&curr->_meta, info))) {
			return MPT_ERROR(BadValue);
		}
		if (!times) {
			times = src;
		} else {
			curr = 0;
		}
	}
	/* require arg on non-existing time info */
	else if (!times) {
		mpt_log(info, __func__, MPT_LOG(Error), "%s",
		        MPT_tr("no default time source"));
		return MPT_ERROR(MissingData);
	}
	/* require valid time source */
	if ((len = times->_vptr->get(times, 'd', &t)) < 0) {
		if (info) {
			const char *src = curr ? MPT_tr("config") : MPT_tr("argument");
			mpt_log(info, __func__, MPT_LOG(Error), "%s (%s)",
			        MPT_tr("bad time iteratior"), src);
		}
		return MPT_ERROR(BadType);
	}
	if (len && (len = times->_vptr->advance(times)) < 0) {
		mpt_log(info, __func__, MPT_LOG(Error), "%s: %s",
		        MPT_tr("unable to advance iterator"), MPT_tr("time steps"));
	}
	/* load user parameter */
	if (!md->npar &&
	    (curr = mpt_node_next(conf, "param"))) {
		if ((len = mpt_conf_param(&md->param, curr, 0)) < 0) {
			if (info) {
				mpt_log(info, __func__, MPT_LOG(Warning), "%s",
				        MPT_tr("invalid parameter data"));
			}
			return len;
		}
		md->npar = len;
	}
	/* clear value buffer */
	if ((buf = md->val._buf)) {
		buf->_used = 0;
	}
	/* setup ODE/DAE mode */
	if (!(curr = mpt_node_next(conf, "grid"))) {
		if (!mpt_array_append(&md->val, sizeof(t), &t)) {
			if (info) {
				mpt_log(info, __func__, MPT_LOG(Error), "%s",
				        MPT_tr("unable to save initial time in history"));
			}
			return MPT_ERROR(BadOperation);
		}
		if (!(curr = mpt_node_next(conf, "profile"))) {
			len = 0;
		}
		/* read profile values */
		else if ((len = mpt_conf_param(&md->val, curr, 1)) < 0) {
			if (info) {
				mpt_log(info, __func__, MPT_LOG(Error), "%s",
				        MPT_tr("failed to reserve initial data"));
			}
			return len;
		}
		md->nval = len + 1;
		
		return 0;
	}
	/* require valid grid */
	if ((len = mpt_conf_grid(&md->val, curr->_meta)) < 0) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s",
			        MPT_tr("unable to get grid data"));
		}
		return MPT_ERROR(BadValue);
	}
	/* minimal grid size */
	if (len < 1) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s: %d",
			        MPT_tr("bad grid interval count"), len);
		}
		return MPT_ERROR(BadValue);
	}
	/* profile settings for PDE */
	if ((curr = mpt_node_next(conf, "profile"))) {
		MPT_INTERFACE(metatype) *mt, *old;
		md->nval = -1;
		if (!(mt = mpt_conf_profiles(md, t, curr, info))) {
			return MPT_ERROR(BadValue);
		}
		if ((old = curr->_meta)) {
			old->_vptr->ref.unref((void *) old);
		}
		curr->_meta = mt;
		if (info) {
			if (old) {
				mpt_log(info, __func__, MPT_LOG(Debug), "%s",
				        MPT_tr("replaced profile source"));
			} else {
				mpt_log(info, __func__, MPT_LOG(Debug), "%s",
				        MPT_tr("created profile source"));
			}
		}
	}
	md->nval = 0;
	
	return len;
}
