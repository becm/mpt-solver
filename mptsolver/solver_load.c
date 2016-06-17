
#include <string.h>

#include "client.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief solver proxy access
 * 
 * Load (or convert) solver via proxy element.
 * Track references via proxy data.
 * 
 * \param pr   proxa data
 * \param conf solver alias/symbol to mpt_library_bind
 * \param log  logging descriptor
 * 
 * \return untracked solver interface instance
 */
/* load solver of specific type */
extern int mpt_solver_load(MPT_STRUCT(proxy) *pr, const char *conf, MPT_INTERFACE(logger) *log)
{
	MPT_STRUCT(proxy) p = MPT_PROXY_INIT;
	MPT_INTERFACE(object) *sol;
	MPT_INTERFACE(metatype) *mt;
	const char *lpath = 0;
	int mode;
	
	if ((mt = mpt_config_get(0, "mpt.prefix.lib", '.', 0))) {
		mt->_vptr->conv(mt, 's', &lpath);
	}
	p._types[0] = MPT_ENUM(TypeSolver);
	p._types[1] = MPT_ENUM(TypeObject);
	if ((mode = mpt_library_bind(&p, conf, lpath, log)) < 0) {
		return mode;
	}
	if (!(mt = p._ref)) {
		if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s",
		                 MPT_tr("no proxy instance pointer"), conf);
		return MPT_ERROR(BadValue);
	}
	/* reference is no metatype */
	if (!strchr(p._types, MPT_ENUM(TypeMeta))) {
		if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s: %s",
		                 MPT_tr("bad proxy type"), conf);
		mt->_vptr->unref(mt);
		return MPT_ERROR(BadType);
	}
	/* get solver descriptor from metatype */
	if (mt->_vptr->conv(mt, MPT_ENUM(TypeSolver), &sol) <= 0) {
		if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s: %s",
		                 MPT_tr("no solver in metatype"), conf);
		mt->_vptr->unref(mt);
		return MPT_ERROR(BadType);
	}
	if (sol) {
		MPT_INTERFACE(metatype) *old;
		MPT_STRUCT(property) prop;
		prop.name = "";
		prop.desc = 0;
		if ((mode = sol->_vptr->property(sol, &prop)) < 0) {
			mode = MPT_ERROR(BadOperation);
			mt->_vptr->unref(mt);
		}
		if ((old = pr->_ref)) {
			old->_vptr->unref(old);
		}
		*pr = p;
		
		return mode;
	}
	mt->_vptr->unref(mt);
	if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s: %s",
	                 MPT_tr("bad solver instance pointer"), conf);
	
	return MPT_ERROR(BadValue);
}
