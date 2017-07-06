/*!
 * set parameter for limex integrator descriptor.
 */

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#include "version.h"

#include "meta.h"

#include "limex.h"

#include "solver_yprime.h"

static int setJacobian(MPT_SOLVER_STRUCT(limex) *lx, const MPT_INTERFACE(metatype) *src)
{
	MPT_STRUCT(solver_value) val;
	int ret, usr, key;
	int32_t lb, ub;
	long max;
	
	if (!src) {
		lx->iopt[6] = 0;
		return 0;
	}
	ret = 1;
	if ((key = mpt_solver_value_set(&val, src)) < 0) {
		const char *par = 0;
		if ((key = src->_vptr->conv(src, 'k', &par)) < 0) {
			return key;
		}
		key = (ret && par) ? *par : 0;
		ret = 0;
	}
	else if ((key = mpt_solver_next_key(&val)) < 0) {
		return key;
	}
	if (!key) {
		lx->iopt[6] = 0;
		return 0;
	}
	max = lx->ivp.neqs * (lx->ivp.pint + 1);
	usr = 0;
	switch (key) {
		case 'F':
			usr = 1;
		case 'f':
			lb = max;
			ub = lb;
			break;
		case 'B':
			usr = 1;
		case 'b':
			if (!ret) {
				lb = ub = lx->ivp.neqs;
			}
			else if ((ret = mpt_solver_next_int(&val, &lb)) < 0) {
				return ret;
			}
			else if (!ret) {
				lb = ub = lx->ivp.neqs;
				ret = 1;
				break;
			}
			else if ((ret = mpt_solver_next_int(&val, &ub)) < 0) {
				return ret;
			}
			else if (!ret) {
				ub = lb;
				ret = 2;
				break;
			}
			else {
				ret = 3;
			}
			break;
		default: 
			return MPT_ERROR(BadValue);
	}
	if (lb < 0 || ub < 0) {
		return MPT_ERROR(BadValue);
	}
	if (lb > max || ub > max) {
		return MPT_ERROR(BadValue);
	}
	lx->iopt[6] = usr;
	lx->iopt[7] = lb;
	lx->iopt[8] = ub;
	return ret;
}

extern int mpt_limex_set(MPT_SOLVER_STRUCT(limex) *lx, const char *name, const MPT_INTERFACE(metatype) *src)
{
	int ret = 0;
	
	/* initial values */
	if (!name) {
		return mpt_solver_ivpstate(&lx->ivp, &lx->t, &lx->y, src);
	}
	/* change solver dimensions, reinit */
	if (!*name) {
		MPT_IVP_STRUCT(parameters) ivp = MPT_IVPPAR_INIT;
		
		if (src && (ret = mpt_solver_ivpset(&ivp, src)) < 0) {
			return ret;
		}
		mpt_limex_fini(lx);
		mpt_limex_init(lx);
		lx->ivp = ivp;
		return ret;
	}
	if (!strcasecmp(name, "atol")) {
		return mpt_solver_settol(&lx->atol, src, __MPT_IVP_ATOL);
	}
	if (!strcasecmp(name, "rtol")) {
		return mpt_solver_settol(&lx->rtol, src, __MPT_IVP_RTOL);
	}
	if (!strncasecmp(name, "jac", 3)) {
		return setJacobian(lx, src);
	}
	if (!strcasecmp(name, "h") || !strcasecmp(name, "initstep")) {
		double val = 0;
		if (src && (ret = src->_vptr->conv(src, 'd', &val)) < 0) {
			return ret;
		}
		if (val < 0) {
			return MPT_ERROR(BadValue);
		}
		lx->h = val;
		return 0;
	}
	/* integer parameter */
	if (!strcasecmp(name, "monitor") || !strcasecmp(name, "iopt1")) {
		int32_t val = 0;
		if (src && (ret = src->_vptr->conv(src, 'i', &val)) < 0) {
			return ret;
		}
		if (val < 0 || val > 2) {
			return MPT_ERROR(BadValue);
		}
		lx->iopt[0] = val;
		return 0;
	}
	if (!strcasecmp(name, "solout") || !strcasecmp(name, "iopt3")) {
		return MPT_ERROR(BadOperation);
	}
	if (!strncasecmp(name, "bnosingular", 4) || !strcasecmp(name, "iopt5")) {
		int32_t val = 0;
		if (src && (ret = src->_vptr->conv(src, 'i', &val)) < 0) {
			return ret;
		}
		if (val < 0 || val > 1) {
			return MPT_ERROR(BadValue);
		}
		lx->iopt[4] = val;
		return 0;
	}
	if (!strcasecmp(name, "jacreuse") || !strcasecmp(name, "iopt10")) {
		int32_t val = 0;
		if (src && (ret = src->_vptr->conv(src, 'i', &val)) < 0) {
			return ret;
		}
		if (val < 0 || val > 1) return MPT_ERROR(BadValue);
		lx->iopt[9] = val;
		return 0;
	}
	if (!strcasecmp(name, "single") || !strcasecmp(name, "iopt12")) {
		int32_t val = 0;
		if (src && (ret = src->_vptr->conv(src, 'i', &val)) < 0) {
			return ret;
		}
		if (val < 0 || val > 1) {
			return MPT_ERROR(BadValue);
		}
		lx->iopt[11] = val;
		return 0;
	}
	if (!strcasecmp(name, "denseout")) {
		return MPT_ERROR(BadOperation);
	}
	if (!strcasecmp(name, "tend") || !strcasecmp(name, "ropt3")) {
		double val;
		if (src && (ret = src->_vptr->conv(src, 'i', &val)) < 0) {
			return ret;
		}
		if (ret) {
			lx->ropt[2] = val;
			lx->iopt[16] = 1;
		} else {
			lx->iopt[16] = 0;
		}
		return 0;
	}
	if (!strcasecmp(name, "plotjac") || !strcasecmp(name, "iopt18")) {
		int32_t val = 0;
		if (src && (ret = src->_vptr->conv(src, 'i', &val)) < 0) {
			return ret;
		}
		if (val < -1) {
			return MPT_ERROR(BadValue);
		}
		lx->iopt[17] = val;
		return 0;
	}
	if (!strcasecmp(name, "maxstep") || !strcasecmp(name, "ropt1")) {
		double val = 0;
		if (src && (ret = src->_vptr->conv(src, 'd', &val)) < 0) {
			return ret;
		}
		if (val < 0) {
			return MPT_ERROR(BadValue);
		}
		lx->ropt[0] = val;
		return 0;
	}
	if (!strncasecmp(name, "yprime", 2) || !strcasecmp(name, "ys")) {
		return _mpt_solver_yprime_set(&lx->ivp, &lx->ys, src);
	}
	return MPT_ERROR(BadArgument);
}


extern int mpt_limex_get(const MPT_SOLVER_STRUCT(limex) *lx, MPT_STRUCT(property) *prop)
{
	const char *name;
	intptr_t pos = -1, id;
	
	if (!prop) {
		return MPT_ENUM(TypeSolver);
	}
	if (!(name = prop->name)) {
		if ((pos = (intptr_t) prop->desc) < 0) {
			return MPT_ERROR(BadArgument);
		}
	}
	else if (!*name) {
		prop->name = "limex"; prop->desc = "extrapolation integrator for linearly-implicit DAE";
		prop->val.fmt = "ii"; prop->val.ptr = &lx->ivp;
		return MPT_SOLVER_ENUM(ODE) | MPT_SOLVER_ENUM(DAE) | MPT_SOLVER_ENUM(PDE);
	}
	else if (!strcasecmp(name, "version")) {
		static const char version[] = BUILD_VERSION"\0";
		prop->name = "version"; prop->desc = "solver release information";
		prop->val.fmt= 0; prop->val.ptr = version;
		return 0;
	}
	
	id = 0;
	if (name ? !strcasecmp(name, "atol") : pos == id++) {
		if (!lx) { prop->val.fmt = "d"; prop->val.ptr = &lx->atol.d.val; }
		else { id = mpt_solver_vecpar_get(&lx->atol, &prop->val); }
		prop->name = "atol"; prop->desc = "absolute tolerances";
		return id;
	}
	if (name ? !strcasecmp(name, "rtol") : pos == id++) {
		if (!lx) { prop->val.fmt = "d"; prop->val.ptr = &lx->rtol.d.val; }
		else { id = mpt_solver_vecpar_get(&lx->rtol, &prop->val); }
		prop->name = "rtol"; prop->desc = "relative tolerances";
		return id;
	}
	if (name ? !strncasecmp(name, "jac", 3) : pos == id++) {
		prop->name = "jacobian"; prop->desc = "(user) jacobian settings";
		prop->val.fmt = "iii"; prop->val.ptr = lx->iopt + 6;
		if (!lx) return id;
		return lx->iopt[6] || lx->iopt[7] != lx->ivp.neqs || lx->iopt[8] != lx->ivp.neqs;
	}
	if (name ? (!strcasecmp(name, "h") || !strcasecmp(name, "initstep")) : pos == id++) {
		prop->name = "h"; prop->desc = "initial/next stepsize";
		prop->val.fmt = "d"; prop->val.ptr = &lx->h;
		if (!lx) return id;
		return lx->h ? 1 : 0;
	}
	/* integer parameter */
	if (name ? (!strcasecmp(name, "monitor") || !strcasecmp(name, "iopt1")) : pos == id++) {
		prop->name = "monitor"; prop->desc = "integration monitor output";
		prop->val.fmt = "i"; prop->val.ptr = lx->iopt;
		if (!lx) return id;
		return lx->iopt[0] ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "solout") || !strcasecmp(name, "iopt3")) : pos == id++) {
		prop->name = "solout"; prop->desc = "(intermediate) solution output";
		prop->val.fmt = "i"; prop->val.ptr = lx->iopt+2;
		if (!lx) return id;
		return lx->iopt[2] ? 1 : 0;
	}
	if (name ? (!strncasecmp(name, "bnos", 4) || !strcasecmp(name, "iopt5")) : pos == id++) {
		prop->name = "bnosingular"; prop->desc = "B-matrix may not be singular";
		prop->val.fmt = "i"; prop->val.ptr = lx->iopt+4;
		if (!lx) return id;
		return lx->iopt[4] ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "jacreuse") || !strcasecmp(name, "iopt10")) : pos == id++) {
		prop->name = "jacreuse"; prop->desc = "try to reuse jacobian";
		prop->val.fmt = "i"; prop->val.ptr = lx->iopt+9;
		if (!lx) return id;
		return lx->iopt[9] ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "single") || !strcasecmp(name, "iopt12")) : pos == id++) {
		prop->name = "single"; prop->desc = "single step mode";
		prop->val.fmt = "i"; prop->val.ptr = lx->iopt+11;
		if (!lx) return id;
		return lx->iopt[11] ? 1 : 0;
	}
	if (name ? !strcasecmp(name, "denseout") : pos == id++) {
		prop->name = "denseout"; prop->desc = "dense output settings";
		prop->val.fmt = "i"; prop->val.ptr = lx->iopt+12;
		if (!lx) return id;
		if (lx->iopt[12] == 1 || lx->iopt[12] == 2) {
			prop->val.fmt = "ii";
			return 1;
		}
		if (lx->iopt[12] == 3) {
			prop->val.fmt = "d";
			prop->val.ptr = lx->ropt+1;
			return 1;
		}
		return 0;
	}
	if (name ? (!strcasecmp(name, "tend") || !strcasecmp(name, "ropt3")) : pos == id++) {
		prop->name = "tend"; prop->desc = "dense output settings";
		prop->val.fmt = "d"; prop->val.ptr = lx->ropt+2;
		if (!lx) return id;
		return lx->iopt[16] ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "plotjac") || !strcasecmp(name, "iopt18")) : pos == id++) {
		prop->name = "plotjac"; prop->desc = "dense output settings";
		prop->val.fmt = "i"; prop->val.ptr = lx->iopt+17;
		if (!lx) return id;
		return lx->iopt[17] != 0;
	}
	if (name ? (!strcasecmp(name, "maxstep") || !strcasecmp(name, "ropt1")) : pos == id++) {
		prop->name = "ipos"; prop->desc = "maximum internal step size";
		prop->val.fmt = "d"; prop->val.ptr = lx->ropt;
		if (!lx) return id;
		return lx->ropt[0] ? 1 : 0;
	}
	/* state properties */
	if (!name || !lx) {
		return MPT_ERROR(BadArgument);
	}
	if (!strcasecmp(name, "ipos")) {
		prop->name = "ipos"; prop->desc = "maximum internal step size";
		prop->val.fmt = "i"; prop->val.ptr = lx->ipos;
		return lx->ivp.neqs * (lx->ivp.pint + 1);
	}
	if (!strncasecmp(name, "yprime", 2) || !strcasecmp(name, "ys")) {
		prop->name = "yprime"; prop->desc = "current deviation vector";
		prop->val.fmt = "d"; prop->val.ptr = lx->ys;
		return lx->ivp.neqs * (lx->ivp.pint + 1);
	}
	return MPT_ERROR(BadArgument);
}
