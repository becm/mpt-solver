
#include <string.h>

#include "client.h"
#include "config.h"

#include "meta.h"

#include "solver.h"

static int validSolver(MPT_SOLVER(interface) *sol, int match, MPT_INTERFACE(logger) *log, const char *fcn)
{
	MPT_INTERFACE(object) *obj;
	const char *name;
	int cap;
	
	obj = 0;
	name = 0;
	if (log
	    && (cap = sol->_vptr->meta.conv((void *) sol, MPT_ENUM(TypeObject), &obj)) >= 0
	    && obj) {
		name = mpt_object_typename(obj);
	}
	if ((cap = sol->_vptr->report(sol, 0, 0, 0)) < 0) {
		int type = sol->_vptr->meta.conv((void *) sol, 0, 0);
		if (log) {
			const char *err = MPT_tr("no valid solver");
			if (name) {
				mpt_log(log, fcn, MPT_LOG(Error), "%s: %s (%d)",
				        err, name, type);
			} else {
				mpt_log(log, fcn, MPT_LOG(Error), "%s: %d",
				        err, type);
			}
		}
		return MPT_ERROR(BadType);
	}
	if (match && !(cap & match)) {
		if (log) {
			const char *err = MPT_tr("solver has incompatile capabilities");
			if (name) {
				mpt_log(log, fcn, MPT_LOG(Error), "%s: 0x%02x <> 0x%02x (%s)",
				        err, cap, match, name);
			} else {
				mpt_log(log, fcn, MPT_LOG(Error), "%s: 0x%02x <> 0x%02x",
				        err, cap, match);
			}
		}
		return MPT_ERROR(BadValue);
	}
	if (log) {
		const char *msg = MPT_tr("use solver instance");
		if (name) {
			mpt_log(log, fcn, MPT_LOG(Info), "%s: %s (0x%02x)",
			        msg, name, cap);
		} else {
			mpt_log(log, fcn, MPT_LOG(Info), "%s: 0x%02x",
			        msg, cap);
		}
	}
	return cap;
}
    
/*!
 * \ingroup mptSolver
 * \brief solver proxy access
 * 
 * Load (or convert) reference in proxy element.
 * Track/Replace reference in proxy structure.
 * 
 * \param pr   proxy data
 * \param conf solver alias/symbol to bind
 * 
 * \return solver interface (tracked by proxy reference)
 */
/* load solver of specific type */
extern MPT_SOLVER(interface) *mpt_solver_load(MPT_STRUCT(proxy) *pr, int match, const char *conf, MPT_INTERFACE(logger) *log)
{
	MPT_INTERFACE(metatype) *mt;
	MPT_SOLVER(interface) *sol;
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
	sol = 0;
	if (!conf) {
		if (!(mt = pr->_ref)) {
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Error), "%s",
			                MPT_tr("no existing solver"));
			}
			return 0;
		}
		if ((mode = mt->_vptr->conv(mt, me, &sol)) < 0
		    || !sol) {
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Error), "%s",
			                MPT_tr("no solver in proxy object"));
			}
			return 0;
		}
		return validSolver(sol, match, log, __func__) < 0 ? 0 : sol;
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
	/* identical to current instance */
	h = mpt_hash(conf, strlen(conf));
	if ((mt = pr->_ref)
	    && pr->_hash == h
	    && (mode = mt->_vptr->conv(mt, me, &sol)) >= 0
	    && sol
	    && (mode = validSolver(sol, match, log, __func__)) >= 0) {
		return sol;
	}
	/* change library symbol */
	else {
		const MPT_INTERFACE(metatype) *cfg;
		MPT_SOLVER(interface) *sol;
		MPT_INTERFACE(metatype) *old;
		const char *lpath = 0;
		if ((cfg = mpt_config_get(0, "mpt.prefix.lib", '.', 0))) {
			cfg->_vptr->conv(cfg, 's', &lpath);
		}
		if (!(mt = mpt_library_bind(conf, lpath, log))) {
			return 0;
		}
		if ((mode = mt->_vptr->conv(mt, me, &sol)) < 0
		    || !sol) {
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Error), "%s: %s",
				        MPT_tr("no solver type"), conf);
			}
			mt->_vptr->ref.unref((void *) mt);
			return 0;
		}
		if ((mode = validSolver(sol, match, log, __func__) < 0)) {
			mode = mt->_vptr->conv(mt, 0, 0);
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Error), "%s: %s (%d)",
				        MPT_tr("invalid solver description"), conf, mode);
			}
			mt->_vptr->ref.unref((void *) mt);
			return 0;
		}
		if ((old = pr->_ref)) {
			old->_vptr->ref.unref((void *) old);
		}
		pr->_ref  = mt;
		pr->_hash = h;
		
		return sol;
	}
}
