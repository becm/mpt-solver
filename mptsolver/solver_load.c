
#include <string.h>
#include <ctype.h>

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
				mpt_log(log, fcn, MPT_LOG(Error), "%s (%s): 0x%02x <> 0x%02x",
				        err, name, cap, match);
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
extern MPT_SOLVER(interface) *mpt_solver_load(MPT_INTERFACE(metatype) **ref, int match, const char *conf, MPT_INTERFACE(logger) *log)
{
	MPT_INTERFACE(metatype) *mt;
	MPT_SOLVER(interface) *sol;
	const char *old;
	int mode, me;
	
	if ((me = mpt_solver_typeid()) < 0) {
		if (log) {
			mpt_log(log, __func__, MPT_LOG(Critical), "%s",
		                MPT_tr("no registration for solver interface"));
		}
		return 0;
	}
	mt = *ref;
	sol = 0;
	if (!conf) {
		if (!mt) {
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
		if (!(conf = mpt_solver_alias(0))) {
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Error), "%s",
				        MPT_tr("no solver description"));
			}
			return 0;
		}
	}
	else if (!strchr(conf, '@')) {
		const char *name = conf;
		char tmp[256];
		size_t pos = 0;
		
		conf = 0;
		while (1) {
			const char *sym;
			int val;
			if (!(val = name[pos])) {
				if (!(sym = mpt_solver_alias(name))) {
					if (log) {
						mpt_log(log, __func__, MPT_LOG(Error), "%s (%d)",
						        MPT_tr("no valid solver alias"), conf);
					}
					return 0;
				}
				conf = sym;
				break;
			}
			if (!isspace(val)) {
				if (pos < sizeof(tmp)) {
					tmp[pos++] = val;
					continue;
				}
				++pos;
				while ((val = name[pos])) {
					if (isspace(val)) {
						break;
					}
					++pos;
				}
				val = pos;
				if (log) {
					mpt_log(log, __func__, MPT_LOG(Error), "%s (%d)",
					        MPT_tr("solver description element exceeds buffer"), val);
				}
				name += pos;
				pos = 0;
			}
			if (!pos) {
				++name;
				continue;
			}
			tmp[pos++] = 0;
			if ((sym = mpt_solver_alias(tmp))) {
				conf = sym;
				break;
			}
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Error), "%s: %s",
				        MPT_tr("loader alias unknown"), tmp);
			}
			name += pos;
			pos = 0;
		}
	}
	/* identical to current instance */
	if (mt
	    && (old = mpt_meta_data(mt, 0))
	    && !strcmp(conf, old)
	    && (mode = mt->_vptr->conv(mt, me, &sol)) >= 0
	    && sol
	    && (mode = validSolver(sol, match, log, __func__)) >= 0) {
		return sol;
	}
	/* change library symbol */
	else {
		const MPT_INTERFACE(metatype) *cfg;
		MPT_SOLVER(interface) *sol;
		MPT_INTERFACE(metatype) *next;
		const char *lpath = 0;
		
		if ((cfg = mpt_config_get(0, "mpt.prefix.lib", '.', 0))) {
			cfg->_vptr->conv(cfg, 's', &lpath);
		}
		if (!(next = mpt_library_meta(me, conf, lpath, log))) {
			return 0;
		}
		sol = 0;
		if ((mode = next->_vptr->conv(next, me, &sol)) < 0
		    || !sol) {
			if (log) {
				mode = next->_vptr->conv(next, 0, 0);
				mpt_log(log, __func__, MPT_LOG(Error), "%s (%d): %s",
				        MPT_tr("no solver type"), mode, conf);
			}
			next->_vptr->ref.unref((void *) next);
			return 0;
		}
		if ((mode = validSolver(sol, match, log, __func__) < 0)) {
			next->_vptr->ref.unref((void *) next);
			return 0;
		}
		if (mt) {
			mt->_vptr->ref.unref((void *) mt);
		}
		*ref = next;
		
		return sol;
	}
}
