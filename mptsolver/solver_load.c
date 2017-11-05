
#include <string.h>

#include "client.h"
#include "config.h"

#include "meta.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief solver proxy access
 * 
 * Load (or convert) solver via proxy element.
 * Track references via proxy data.
 * 
 * \param pr   proxy data
 * \param conf solver alias/symbol to mpt_library_bind
 * 
 * \return untracked solver interface instance
 */
/* load solver of specific type */
extern MPT_SOLVER(interface) *mpt_solver_load(MPT_STRUCT(proxy) *pr, int match, const char *conf, MPT_INTERFACE(logger) *log)
{
	MPT_SOLVER(interface) *sol;
	MPT_INTERFACE(metatype) *mt;
	const char *name;
	uintptr_t h;
	int mode, me;
	
	if ((me = mpt_solver_typeid()) < 0) {
		if (log) {
			mpt_log(log, __func__, MPT_LOG(Critical), "%s",
		                MPT_tr("no registration for solver interface"));
		}
		return 0;
	}
	if (!conf) {
		MPT_INTERFACE(object) *obj;
		if (!(mt = pr->_ref)) {
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Error), "%s",
			                MPT_tr("no existing solver"));
			}
			return 0;
		}
		sol = 0;
		if (mt->_vptr->conv(mt, me, &sol) < 0
		    || !sol) {
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Error), "%s: %s",
			                MPT_tr("no solver in proxy instance"), conf);
			}
			return 0;
		}
		obj = 0;
		name = 0;
		if ((mode = mt->_vptr->conv(mt, MPT_ENUM(TypeObject), &obj)) >= 0
		    && obj) {
			name = mpt_object_typename(obj);
		}
		if ((mode = sol->_vptr->report(sol, 0, 0, 0)) < 0) {
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Error), "%s: %s",
			                MPT_tr("no valid solver"), name ? name : "");
			}
			return 0;
		}
		if (match && !(mode & match)) {
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Error), "%s: 0x%02x <> 0x%02x",
			                MPT_tr("solver has invalid type"), mode, match);
			}
			return 0;
		}
		return sol;
	}
	if (!*conf) {
		if (log) {
			mpt_log(log, __func__, MPT_LOG(Error), "%s",
		                MPT_tr("no solver description"));
		}
		return 0;
	}
	if (!strchr(conf, '@')) {
		if (!(name = mpt_solver_alias(conf))) {
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Error), "%s: %s",
			                MPT_tr("bad solver description"), conf);
			}
			return 0;
		}
		conf = name;
	}
	/* identical to current creator */
	h = mpt_hash(conf, strlen(conf));
	if (pr->_hash
	    && pr->_hash == h) {
		mt = pr->_ref;
	}
	/* change library symbol */
	else {
		const MPT_INTERFACE(metatype) *cfg;
		const char *lpath = 0;
		if ((cfg = mpt_config_get(0, "mpt.prefix.lib", '.', 0))) {
			cfg->_vptr->conv(cfg, 's', &lpath);
		}
		if (!(mt = mpt_library_bind(conf, lpath, log))) {
			return 0;
		}
	}
	if (mt->_vptr->conv(mt, me, &sol) < 0) {
		if (log) {
			mpt_log(log, __func__, MPT_LOG(Error), "%s: %s",
		                MPT_tr("no solver in proxy instance"), conf);
		}
		return 0;
	}
	if (sol) {
		MPT_INTERFACE(metatype) *pre;
		
		if ((mode = sol->_vptr->report(sol, 0, 0, 0)) < 0) {
			mode = MPT_ERROR(BadOperation);
			mt->_vptr->ref.unref((void *) mt);
			return 0;
		}
		if (match && !(mode & match)) {
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Error), "%s (0x%x <> 0x%x): %s",
			                MPT_tr("incompatible solver type"), mode, match, conf);
			}
			mt->_vptr->ref.unref((void *) mt);
			return 0;
		}
		pre = pr->_ref;
		
		/* replace proxy target */
		if (mt != pre) {
			if (pre) {
				pre->_vptr->ref.unref((void *) pre);
			}
			pr->_ref = mt;
			pr->_hash = h;
		}
		return sol;
	}
	/* created instance not compatible */
	if (mt != pr->_ref) {
		mt->_vptr->ref.unref((void *) mt);
	}
	if (log) {
		mpt_log(log, __func__, MPT_LOG(Error), "%s: %s",
	                MPT_tr("bad solver instance pointer"), conf);
	}
	return 0;
}
