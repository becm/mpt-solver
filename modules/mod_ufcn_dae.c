/*!
 * MPT solver module helper function
 *   assign DAE functions according to parameter
 */

#include "../solver.h"

static int fcnFlags(long pint, const MPT_IVP_STRUCT(daefcn) *ufcn)
{
	int ret = 0;
	if (pint) {
		ret |= MPT_SOLVER_ENUM(PDE);
	}
	if (ufcn->jac.fcn) {
		ret |= MPT_SOLVER_ENUM(IvpJac);
	}
	if (ufcn->rside.fcn) {
		ret |= MPT_SOLVER_ENUM(IvpRside);
	}
	if (ufcn->mas.fcn) {
		ret |= MPT_SOLVER_ENUM(IvpMas);
	}
	return ret;
}

extern int mpt_solver_module_ufcn_dae(long pint, MPT_IVP_STRUCT(daefcn) *ufcn, int type, const void *ptr)
{
	if (!ufcn) {
		return pint ? MPT_SOLVER_ENUM(PDE) : 0;
	}
	if (!ptr) {
		if ((type & MPT_SOLVER_ENUM(IvpRside))) {
			ufcn->rside.fcn = 0;
		}
		if ((type & MPT_SOLVER_ENUM(IvpJac))) {
			ufcn->jac.fcn = 0;
		}
		if ((type & MPT_SOLVER_ENUM(IvpMas))) {
			ufcn->mas.fcn = 0;
		}
		return fcnFlags(pint, ufcn);
	}
	if (pint) {
		if (!(type & MPT_SOLVER_ENUM(PDE))) {
			return MPT_ERROR(BadType);
		}
		type &= ~MPT_SOLVER_ENUM(PDE);
	}
	else if (type & MPT_SOLVER_ENUM(PDE)) {
		return MPT_ERROR(BadType);
	}
	switch (type) {
	  case MPT_SOLVER_ENUM(IvpRside):
		if (!((MPT_IVP_STRUCT(rside) *) ptr)->fcn) {
			return MPT_ERROR(BadValue);
		}
		ufcn->rside = *((MPT_IVP_STRUCT(rside) *) ptr);
		break;
	  case MPT_SOLVER_ENUM(IvpJac):
		if (!((MPT_IVP_STRUCT(jacobian) *) ptr)->fcn) {
			return MPT_ERROR(BadValue);
		}
		ufcn->jac = *((MPT_IVP_STRUCT(jacobian) *) ptr);
		break;
	  case MPT_SOLVER_ENUM(ODE):
		if (!((MPT_IVP_STRUCT(odefcn) *) ptr)->rside.fcn) {
			return MPT_ERROR(BadValue);
		}
		ufcn->rside = ((MPT_IVP_STRUCT(odefcn) *) ptr)->rside;
		ufcn->jac   = ((MPT_IVP_STRUCT(odefcn) *) ptr)->jac;
		break;
	  case MPT_SOLVER_ENUM(DAE):
		if (!((MPT_IVP_STRUCT(daefcn) *) ptr)->rside.fcn) {
			return MPT_ERROR(BadValue);
		}
		*ufcn = *((MPT_IVP_STRUCT(daefcn) *) ptr);
		break;
	  default:
		return MPT_ERROR(BadType);
	}
	return fcnFlags(pint, ufcn);
}

