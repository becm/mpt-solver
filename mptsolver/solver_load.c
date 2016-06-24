
#include <string.h>

#include "client.h"

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
extern int mpt_solver_load(MPT_STRUCT(proxy) *pr, const char *conf)
{
	MPT_INTERFACE(object) *sol;
	MPT_INTERFACE(metatype) *mt;
	const char *lpath = 0;
	uintptr_t h;
	int mode;
	
	if (!conf) {
		if (pr->logger) mpt_log(pr->logger, __func__, MPT_FCNLOG(Error), "%s",
		                        MPT_tr("no solver description"));
		return MPT_ERROR(BadArgument);
	}
	if (pr->hash
	    && pr->hash == (h = mpt_hash(conf, strlen(conf)))) {
		mt = pr->_mt;
	}
	else if ((mt = mpt_config_get(0, "mpt.prefix.lib", '.', 0))) {
		mt->_vptr->conv(mt, 's', &lpath);
	}
	if (!(mt = mpt_library_bind(MPT_ENUM(TypeSolver), conf, lpath, pr->logger))) {
		return 0;
	}
	if (mt->_vptr->conv(mt, MPT_ENUM(TypeSolver), &sol) < 0) {
		if (pr->logger) mpt_log(pr->logger, __func__, MPT_FCNLOG(Error), "%s: %s",
		                        MPT_tr("no solver in proxy instance"), conf);
		return MPT_ERROR(BadValue);
	}
	if (sol) {
		MPT_STRUCT(property) prop;
		prop.name = "";
		prop.desc = 0;
		if ((mode = sol->_vptr->property(sol, &prop)) < 0) {
			mode = MPT_ERROR(BadOperation);
			mt->_vptr->unref(mt);
		}
		if (pr->_mt == mt) {
			return mode;
		}
		if (pr->_mt) {
			pr->_mt->_vptr->unref(pr->_mt);
		}
		pr->_mt = mt;
		pr->hash = mpt_hash(conf, strlen(conf));
		
		return mode;
	}
	mt->_vptr->unref(mt);
	if (pr->_mt == mt) {
		pr->_mt = 0;
		pr->hash = 0;
	}
	if (pr->logger) mpt_log(pr->logger, __func__, MPT_FCNLOG(Error), "%s: %s",
	                        MPT_tr("bad solver instance pointer"), conf);
	
	return MPT_ERROR(BadValue);
}
