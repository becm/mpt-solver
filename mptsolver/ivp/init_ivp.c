/*!
 * MPT solver library
 *   initialize ODE/DAE problem size and user functions
 */

#include <inttypes.h>
#include <sys/uio.h>

#include "meta.h"
#include "output.h"

#include "solver.h"

static int set_neqs(MPT_INTERFACE(object) *obj, const char *_func, int neqs, MPT_INTERFACE(logger) *info)
{

	if (!obj) {
		if (info) {
			mpt_log(info, _func, MPT_LOG(Error), "%s",
			        MPT_tr("no object interface for solver"));
		}
		return MPT_ERROR(BadValue);
	}
	if ((neqs = mpt_object_set(obj, "", "i", (int32_t) neqs)) < 0) {
		if (info) {
			mpt_log(info, _func, MPT_LOG(Error), "%s",
			        MPT_tr("unable to save problem parameter to solver"));
		}
	}
	return neqs;
}
static int set_fcns(MPT_SOLVER(interface) *sol, const void *fcn, int type, MPT_INTERFACE(logger) *info, const char *_func)
{
	int ret;
	
	/* set user funtions according to type */
	if (!fcn) {
		type = 0;
	}
	else if ((ret = sol->_vptr->setFunctions(sol, type, fcn)) < 0) {
		if (info) {
			mpt_log(info, _func, MPT_LOG(Error), "%s (0x%x)",
			        MPT_tr("unable to set user functions"), type);
		}
		return ret;
	}
	/* limit user functions to compatible types */
	if ((ret = sol->_vptr->setFunctions(sol, ~type, 0)) < 0) {
		if (info) {
			mpt_log(info, _func, MPT_LOG(Error), "%s (0x%x)",
			        MPT_tr("unable to limit user functions"), type);
		}
	}
	return ret;
}
/*!
 * \ingroup mptSolver
 * \brief initialize DAE state
 * 
 * Initial solver setup for DAE problems.
 * 
 * \param mt    solver metatype
 * \param fcn   DAE user functions
 * \param neqs  number of equotations
 * \param info  log/error output descriptor
 * 
 * \return init result
 */
extern int mpt_init_dae(const MPT_INTERFACE(metatype) *mt, const MPT_IVP_STRUCT(daefcn) *fcn, int neqs, MPT_INTERFACE(logger) *info)
{
	MPT_SOLVER(interface) *sol;
	MPT_INTERFACE(object) *obj;
	
	if (fcn && !fcn->rside.fcn) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s",
			        MPT_tr("missing user right side"));
		}
		return MPT_ERROR(BadArgument);
	}
	sol = 0;
	if (mt->_vptr->conv(mt, MPT_ENUM(TypeSolver), &sol) < 0
	    || !sol) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s (%" PRIxPTR ")",
			        MPT_tr("failed to get solver interface"), mt);
		}
		return MPT_ERROR(BadType);
	}
	obj = 0;
	if (mt->_vptr->conv(mt, MPT_ENUM(TypeObject), &obj) >= 0) {
		if ((neqs = set_neqs(obj, __func__, neqs, info)) < 0) {
			return neqs;
		}
	}
	return set_fcns(sol, fcn, MPT_SOLVER_ENUM(DAE), info, __func__);
}
/*!
 * \ingroup mptSolver
 * \brief initialize ODE solver
 * 
 * Initial solver setup for ODE problems.
 * 
 * \param mt    solver metatype
 * \param fcn   ODE user functions
 * \param neqs  number of equotations
 * \param info  log/error output descriptor
 * 
 * \return init result
 */
extern int mpt_init_ode(const MPT_INTERFACE(metatype) *mt, const MPT_IVP_STRUCT(odefcn) *fcn, int neqs, MPT_INTERFACE(logger) *info)
{
	MPT_SOLVER(interface) *sol;
	MPT_INTERFACE(object) *obj;
	
	if (fcn && !fcn->rside.fcn) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s",
			        MPT_tr("missing user right side"));
		}
		return MPT_ERROR(BadArgument);
	}
	sol = 0;
	if (mt->_vptr->conv(mt, MPT_ENUM(TypeSolver), &sol) < 0
	    || !sol) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s (%" PRIxPTR ")",
			        MPT_tr("failed to get solver interface"), mt);
		}
		return MPT_ERROR(BadType);
	}
	obj = 0;
	if (mt->_vptr->conv(mt, MPT_ENUM(TypeObject), &obj) >= 0) {
		if ((neqs = set_neqs(obj, __func__, neqs, info)) < 0) {
			return neqs;
		}
	}
	return set_fcns(sol, fcn, MPT_SOLVER_ENUM(ODE), info, __func__);
}
