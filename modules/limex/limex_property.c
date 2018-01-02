/*!
 * set parameter for limex integrator descriptor.
 */

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#include "version.h"

#include "limex.h"

#include "module_functions.h"

static int setJacobian(MPT_SOLVER_STRUCT(limex) *lx, const MPT_INTERFACE(metatype) *src)
{
	MPT_STRUCT(consumable) val;
	const char *key;
	int ret, usr;
	int32_t lb, ub;
	long max;
	char mode;
	
	if (!src) {
		lx->iopt[6] = 0;
		return 0;
	}
	key = 0;
	if ((ret = mpt_consumable_setup(&val, src)) < 0) {
		if ((ret = src->_vptr->conv(src, 'k', &key)) < 0) {
			return ret;
		}
		ret = 0;
	}
	else if ((ret = mpt_consume_key(&val, &key)) < 0) {
		return ret;
	} else {
		ret = 1;
	}
	if (!key || !(mode = *key)) {
		lx->iopt[6] = 0;
		return ret;
	}
	max = lx->ivp.neqs * (lx->ivp.pint + 1);
	usr = 0;
	switch (mode) {
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
			else if ((ret = mpt_consume_int(&val, &lb)) < 0) {
				return ret;
			}
			else if (!ret) {
				lb = ub = lx->ivp.neqs;
				ret = 1;
				break;
			}
			else if ((ret = mpt_consume_int(&val, &ub)) < 0) {
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
		return MPT_SOLVER_MODULE_FCN(ivp_state)(&lx->ivp, &lx->t, &lx->y, src);
	}
	/* change solver dimensions, reinit */
	if (!*name) {
		MPT_IVP_STRUCT(parameters) ivp = MPT_IVPPAR_INIT;
		
		if (src && (ret = mpt_solver_module_ivpset(&ivp, src)) < 0) {
			return ret;
		}
		mpt_limex_fini(lx);
		mpt_limex_init(lx);
		lx->ivp = ivp;
		return ret;
	}
	if (!strcasecmp(name, "atol")) {
		return mpt_solver_module_tol_set(&lx->atol, src, __MPT_IVP_ATOL);
	}
	if (!strcasecmp(name, "rtol")) {
		return mpt_solver_module_tol_set(&lx->rtol, src, __MPT_IVP_RTOL);
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
		return MPT_SOLVER_MODULE_FCN(ivp_vecset)(&lx->ivp, &lx->ys, src);
	}
	return MPT_ERROR(BadArgument);
}


extern int mpt_limex_get(const MPT_SOLVER_STRUCT(limex) *lx, MPT_STRUCT(property) *prop)
{
	const char *name;
	intptr_t pos = -1, id;
	
	if (!prop) {
		return MPT_SOLVER_ENUM(DAE) | MPT_SOLVER_ENUM(PDE);
	}
	if (!(name = prop->name)) {
		if ((pos = (intptr_t) prop->desc) < 0) {
			return MPT_ERROR(BadArgument);
		}
	}
	else if (!*name) {
		prop->name = "limex";
		prop->desc = "extrapolation integrator for linearly-implicit DAE";
		return mpt_solver_module_value_ivp(&prop->val, lx ? &lx->ivp : 0);
	}
	else if (!strcasecmp(name, "version")) {
		static const char version[] = BUILD_VERSION"\0";
		prop->name = "version";
		prop->desc = "solver release information";
		prop->val.fmt = 0;
		prop->val.ptr = version;
		return 0;
	}
	
	id = 0;
	if (name ? !strcasecmp(name, "atol") : pos == id++) {
		prop->name = "atol";
		prop->desc = "absolute tolerances";
		if (lx) {
			return mpt_solver_module_tol_get(&prop->val, &lx->atol);
		}
		mpt_solver_module_value_double(&prop->val, &lx->atol.d.val);
		return id;
	}
	if (name ? !strcasecmp(name, "rtol") : pos == id++) {
		prop->name = "rtol";
		prop->desc = "relative tolerances";
		if (lx) {
			return mpt_solver_module_tol_get(&prop->val, &lx->rtol);
		}
		mpt_solver_module_value_double(&prop->val, &lx->rtol.d.val);
		return id;
	}
	if (name ? !strncasecmp(name, "jac", 3) : pos == id++) {
		static const uint8_t fmt[] = "iii";
		prop->name = "jacobian";
		prop->desc = "(user) jacobian settings";
		prop->val.fmt = fmt;
		prop->val.ptr = lx->iopt + 6;
		if (!lx) return id;
		return lx->iopt[6] || lx->iopt[7] != lx->ivp.neqs || lx->iopt[8] != lx->ivp.neqs;
	}
	if (name ? (!strcasecmp(name, "h") || !strcasecmp(name, "initstep")) : pos == id++) {
		prop->name = "h";
		prop->desc = "initial/next stepsize";
		mpt_solver_module_value_double(&prop->val, &lx->h);
		if (!lx) return id;
		return lx->h ? 1 : 0;
	}
	/* integer parameter */
	if (name ? (!strcasecmp(name, "monitor") || !strcasecmp(name, "iopt1")) : pos == id++) {
		prop->name = "monitor";
		prop->desc = "integration monitor output";
		mpt_solver_module_value_int(&prop->val, &lx->iopt[0]);
		if (!lx) return id;
		return lx->iopt[0] ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "solout") || !strcasecmp(name, "iopt3")) : pos == id++) {
		prop->name = "solout";
		prop->desc = "(intermediate) solution output";
		mpt_solver_module_value_int(&prop->val, &lx->iopt[2]);
		if (!lx) return id;
		return lx->iopt[2] ? 1 : 0;
	}
	if (name ? (!strncasecmp(name, "bnos", 4) || !strcasecmp(name, "iopt5")) : pos == id++) {
		prop->name = "bnosingular";
		prop->desc = "B-matrix may not be singular";
		mpt_solver_module_value_int(&prop->val, &lx->iopt[4]);
		if (!lx) return id;
		return lx->iopt[4] ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "jacreuse") || !strcasecmp(name, "iopt10")) : pos == id++) {
		prop->name = "jacreuse";
		prop->desc = "try to reuse jacobian";
		mpt_solver_module_value_int(&prop->val, &lx->iopt[9]);
		if (!lx) return id;
		return lx->iopt[9] ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "single") || !strcasecmp(name, "iopt12")) : pos == id++) {
		prop->name = "single";
		prop->desc = "single step mode";
		mpt_solver_module_value_int(&prop->val, &lx->iopt[11]);
		if (!lx) return id;
		return lx->iopt[11] ? 1 : 0;
	}
	if (name ? !strcasecmp(name, "denseout") : pos == id++) {
		prop->name = "denseout";
		prop->desc = "dense output settings";
		mpt_solver_module_value_int(&prop->val, &lx->iopt[12]);
		if (!lx) return id;
		if (lx->iopt[12] == 1 || lx->iopt[12] == 2) {
			return 2;
		}
		if (lx->iopt[12] == 3) {
			mpt_solver_module_value_double(&prop->val, &lx->ropt[1]);
			return 1;
		}
		return 0;
	}
	if (name ? (!strcasecmp(name, "tend") || !strcasecmp(name, "ropt3")) : pos == id++) {
		prop->name = "tend";
		prop->desc = "dense output settings";
		mpt_solver_module_value_double(&prop->val, &lx->ropt[2]);
		if (!lx) return id;
		return lx->iopt[16] ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "plotjac") || !strcasecmp(name, "iopt18")) : pos == id++) {
		prop->name = "plotjac";
		prop->desc = "dense output settings";
		mpt_solver_module_value_int(&prop->val, &lx->iopt[17]);
		if (!lx) return id;
		return lx->iopt[17] != 0;
	}
	if (name ? (!strcasecmp(name, "maxstep") || !strcasecmp(name, "ropt1")) : pos == id++) {
		prop->name = "ipos";
		prop->desc = "maximum internal step size";
		mpt_solver_module_value_double(&prop->val, &lx->ropt[0]);
		if (!lx) return id;
		return lx->ropt[0] ? 1 : 0;
	}
	/* state properties */
	if (!name || !lx) {
		return MPT_ERROR(BadArgument);
	}
	if (!strcasecmp(name, "ipos")) {
		prop->name = "ipos";
		prop->desc = "maximum internal step size";
		mpt_solver_module_value_int(&prop->val, lx->ipos);
		id = lx->ivp.pint + 1;
		return lx->ivp.neqs * id;
	}
	if (!strncasecmp(name, "yprime", 2) || !strcasecmp(name, "ys")) {
		prop->name = "yprime";
		prop->desc = "current deviation vector";
		mpt_solver_module_value_double(&prop->val, lx->ys);
		id = lx->ivp.pint + 1;
		return lx->ivp.neqs * id;
	}
	return MPT_ERROR(BadArgument);
}
