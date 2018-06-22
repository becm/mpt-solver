/*!
 * MPT solver library
 *   get solver interface from metatype
 */

#include <string.h>
#include <ctype.h>

#include "client.h"
#include "config.h"

#include "meta.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief solver access
 * 
 * Convert metatype element to compatible solver type interface.
 * 
 * \param mt     metatype reference
 * \param match  solver capablities to select
 * \param info   log/error output descriptor
 * 
 * \return solver interface
 */
extern MPT_SOLVER(interface) *mpt_solver_conv(const MPT_INTERFACE(metatype) *mt, int match, MPT_INTERFACE(logger) *info)
{
	MPT_SOLVER(interface) *sol;
	MPT_INTERFACE(object) *obj;
	const char *name, *msg;
	int cap;
	
	/* object interface for name query */
	obj = 0;
	name = 0;
	if (info
	    && (cap = mt->_vptr->conv(mt, MPT_type_pointer(MPT_ENUM(TypeObject)), &obj)) >= 0
	    && obj) {
		name = mpt_object_typename(obj);
	}
	/* can convert to solver interface */
	if (!mt
	    || (cap = mt->_vptr->conv(mt, MPT_type_pointer(MPT_ENUM(TypeSolver)), &sol)) < 0
	    || !sol) {
		if (!info) {
			return 0;
		}
		msg = MPT_tr("no valid solver");
		cap = mt->_vptr->conv(mt, 0, 0);
		if (name) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s: %s (%d)",
			        msg, name, cap);
		} else {
			mpt_log(info, __func__, MPT_LOG(Error), "%s: %d",
			        msg, cap);
		}
		return 0;
	}
	/* query solver capabilities */
	if ((cap = sol->_vptr->report(sol, 0, 0, 0)) < 0) {
		if (!info) {
			return 0;
		}
		msg = MPT_tr("no solver capabilities");
		cap = mt->_vptr->conv(mt, 0, 0);
		if (name) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s: %s (%d)",
			        msg, name, cap);
		} else {
			mpt_log(info, __func__, MPT_LOG(Error), "%s: %d",
			        msg, cap);
		}
		return 0;
	}
	if (match && !(cap & match)) {
		if (!info) {
			return 0;
		}
		msg = MPT_tr("solver has incompatile capabilities");
		if (name) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s (%s): 0x%02x <> 0x%02x",
			        msg, name, cap, match);
		} else {
			mpt_log(info, __func__, MPT_LOG(Error), "%s: 0x%02x <> 0x%02x",
			        msg, cap, match);
		}
		return 0;
	}
	if (!info) {
		return sol;
	}
	msg = MPT_tr("use solver instance");
	if (name) {
		mpt_log(info, __func__, MPT_LOG(Info), "%s: %s (0x%02x)",
		        msg, name, cap);
	} else {
		mpt_log(info, __func__, MPT_LOG(Info), "%s: 0x%02x",
		        msg, cap);
	}
	return sol;
}
