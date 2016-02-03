
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
 * \param type solver type to expect
 * \param log  logging descriptor
 * 
 * \return untracked solver interface instance
 */
/* load solver of specific type */
extern MPT_SOLVER_INTERFACE *mpt_solver_load(MPT_STRUCT(proxy) *pr, const char *conf, int type, MPT_INTERFACE(logger) *log)
{
	MPT_SOLVER_INTERFACE *sol;
	MPT_STRUCT(property) prop;
	int mode;
	
	if (!conf) {
		if (!pr->_ref) {
			if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s",
			                 MPT_tr("missing solver description"));
			return 0;
		}
	}
	else {
		MPT_INTERFACE(metatype) *mt;
		const char *lpath = 0;
		
		if ((mt = mpt_config_get(0, "mpt.prefix.lib", '.', 0))) {
			mt->_vptr->conv(mt, 's', &lpath);
		}
		if (mpt_library_bind(pr, MPT_ENUM(TypeSolver), conf, lpath, log) < 0) {
			return 0;
		}
	}
	/* reference is compatible with solver interface */
	if (strchr(pr->_types, MPT_ENUM(TypeSolver))) {
		sol = pr->_ref;
	}
	/* reference is no metatype */
	else if (!strchr(pr->_types, MPT_ENUM(TypeMeta))) {
		if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s: %s",
		                 MPT_tr("bad proxy type"), conf);
		return 0;
	}
	/* get solver descriptor from metatype */
	else {
		MPT_INTERFACE(metatype) *m = pr->_ref;
		if (m->_vptr->conv(m, MPT_ENUM(TypeSolver), &sol) <= 0 || !sol) {
			if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s: %s",
			                 MPT_tr("no solver in metatype"), conf);
			return 0;
		}
	}
	conf = mpt_object_typename((void *) sol);
	
	prop.name = "";
	prop.desc = 0;
	if ((mode = sol->_vptr->obj.property((void*) sol, &prop)) < 0) {
		mode = MPT_ERROR(BadOperation);
	}
	/* any supported type is accepted */
	else if (type < 0) {
		type = -type;
		if (!(type & mode)) {
			mode = MPT_ERROR(BadValue);
		}
	}
	/* all types must be accepted */
	else if ((type & mode) != type) {
		mode = MPT_ERROR(BadValue);
	}
	
	if (mode < 0) {
		if (!conf) conf = "solver";
		switch (type) {
		  case MPT_SOLVER_ENUM(ODE):
			if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s: %s", conf, MPT_tr("unable to handle ODE problem")); break;
		  case MPT_SOLVER_ENUM(DAE):
			if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s: %s", conf, MPT_tr("unable to handle DAE problem")); break;
		  case MPT_SOLVER_ENUM(PDE):
			if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s: %s", conf, MPT_tr("unable to handle PDE problem")); break;
		  case MPT_SOLVER_ENUM(CapableNls):
			if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s: %s", conf, MPT_tr("unable to handle nonlinear problem")); break;
		  default:;
		}
		return 0;
	}
	return sol;
}
