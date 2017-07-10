/*!
 * generic user functions dDASSL solver instance
 */

extern int MPT_SOLVER_MODULE_FCN(ufcn_dae)(long pint, MPT_IVP_STRUCT(daefcn) *ufcn, int type, const void *ptr)
{
	int ret;
	
	if (!ufcn) {
		return pint ? MPT_SOLVER_ENUM(PDE) : 0;
	}
	if (!ptr) {
		if (!(type & MPT_SOLVER_ENUM(IvpRside))
		 || !(type & MPT_SOLVER_ENUM(PDE))) {
			ufcn->rside.fcn = 0;
		}
		if (!(type & MPT_SOLVER_ENUM(IvpJac))) {
			ufcn->jac.fcn = 0;
		}
		if (!(type & MPT_SOLVER_ENUM(IvpMas))) {
			ufcn->mas.fcn = 0;
		}
	}
	else switch (type) {
	  case MPT_SOLVER_ENUM(IvpRside) | MPT_SOLVER_ENUM(PDE):
		if (!pint) {
			return MPT_ERROR(BadType);
		}
		if (!((MPT_IVP_STRUCT(rside) *) ptr)->fcn) {
			return MPT_ERROR(BadValue);
		}
		ufcn->rside = *((MPT_IVP_STRUCT(rside) *) ptr);
		break;
	  case MPT_SOLVER_ENUM(IvpRside):
		if (pint) {
			return MPT_ERROR(BadType);
		}
		if (!((MPT_IVP_STRUCT(rside) *) ptr)->fcn) {
			return MPT_ERROR(BadValue);
		}
		ufcn->rside = *((MPT_IVP_STRUCT(rside) *) ptr);
		break;
	  case MPT_SOLVER_ENUM(IvpJac):
		ufcn->jac = *((MPT_IVP_STRUCT(jacobian) *) ptr);
		break;
	  case MPT_SOLVER_ENUM(ODE):
		ufcn->rside = ((MPT_IVP_STRUCT(odefcn) *) ptr)->rside;
		ufcn->jac   = ((MPT_IVP_STRUCT(odefcn) *) ptr)->jac;
		break;
	  case MPT_SOLVER_ENUM(DAE):
		*ufcn = *((MPT_IVP_STRUCT(daefcn) *) ptr);
		break;
	  default:
		return MPT_ERROR(BadType);
	}
	ret = 0;
	if (ufcn->mas.fcn) {
		ret |= MPT_SOLVER_ENUM(IvpMas);
	}
	if (ufcn->jac.fcn) {
		ret |= MPT_SOLVER_ENUM(IvpJac);
	}
	if (ufcn->rside.fcn) {
		ret |= MPT_SOLVER_ENUM(IvpRside);
	}
	return ret;
}

