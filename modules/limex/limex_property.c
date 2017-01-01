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

static int setJacobian(MPT_SOLVER_STRUCT(limex) *lx, MPT_INTERFACE(metatype) *src)
{
	char *key;
	int len, res, usr;
	int32_t lb, ub;
	
	if (!src) {
		lx->iopt[6] = 0;
		return 0;
	}
	if ((res = src->_vptr->conv(src, 'k' | MPT_ENUM(ValueConsume), &key)) < 0) {
		return res;
	}
	if (!res || !key) {
		lx->iopt[6] = 0;
		return 0;
	}
	usr = 0;
	len = 1;
	switch (key[0]) {
		case 'F':
			usr = 1;
		case 'f':
			lb = lx->ivp.neqs * (lx->ivp.pint + 1);
			ub = lb;
			break;
		case 'B':
			usr = 1;
		case 'b':
			if ((res = src->_vptr->conv(src, 'i' | MPT_ENUM(ValueConsume), &lb)) < 0) {
				return res;
			}
			if (!res) {
				lb = lx->ivp.neqs;
				ub = lb;
				break;
			}
			++len;
			if ((res = src->_vptr->conv(src, 'i' | MPT_ENUM(ValueConsume), &ub)) < 0) {
				return res;
			}
			if (!res) {
				ub = lb;
				break;
			}
			++len;
			break;;
		default: 
			return MPT_ERROR(BadValue);
	}
	lx->iopt[6] = usr;
	lx->iopt[7] = lb;
	lx->iopt[8] = ub;
	return len;
}

extern int mpt_limex_set(MPT_SOLVER_STRUCT(limex) *lx, const char *name, MPT_INTERFACE(metatype) *src)
{
	int ret = 0;
	
	/* initial values */
	if (!name) {
		double t = lx->t;
		size_t required;
		
		if (src && (ret = src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), &t) <= 0)) {
			if (ret < 0) {
				return ret;
			}
			src = 0;
		}
		required = lx->ivp.neqs * (lx->ivp.pint + 1);
		if ((ret = mpt_vecpar_set(&lx->y, required, src)) < 0) {
			return ret;
		}
		lx->t = t;
		
		return src ? ++ret : 0;
	}
	/* change solver dimensions, reinit */
	if (!*name) {
		MPT_SOLVER_IVP_STRUCT(parameters) ivp = lx->ivp;
		int ret = 0;
		
		if (src && (ret = mpt_ivppar_set(&ivp, src)) < 0) {
			return ret;
		}
		mpt_limex_reset(lx);
		lx->ivp = ivp;
		return ret;
	}
	if (!strcasecmp(name, "atol")) {
		return mpt_vecpar_settol(&lx->atol, src, __MPT_IVP_ATOL);
	}
	if (!strcasecmp(name, "rtol")) {
		return mpt_vecpar_settol(&lx->rtol, src, __MPT_IVP_RTOL);
	}
	if (!strncasecmp(name, "jac", 3)) {
		return setJacobian(lx, src);
	}
	if (!strcasecmp(name, "h") || !strcasecmp(name, "initstep")) {
		double val = 0;
		if (src && (ret = src->_vptr->conv(src, 'd', &val)) < 0) return ret;
		if (val < 0) return MPT_ERROR(BadValue);
		lx->h = val;
		return ret ? 1 : 0;
	}
	/* integer parameter */
	if (!strcasecmp(name, "monitor") || !strcasecmp(name, "iopt1")) {
		int32_t val = 0;
		if (src && (ret = src->_vptr->conv(src, 'i', &val)) < 0) return ret;
		if (val < 0 || val > 2) return MPT_ERROR(BadValue);
		lx->iopt[0] = val;
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "solout") || !strcasecmp(name, "iopt3")) {
		return MPT_ERROR(BadOperation);
	}
	if (!strncasecmp(name, "bnosingular", 4) || !strcasecmp(name, "iopt5")) {
		int32_t val = 0;
		if (src && (ret = src->_vptr->conv(src, 'i', &val)) < 0) return ret;
		if (val < 0 || val > 1) return MPT_ERROR(BadValue);
		lx->iopt[4] = val;
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "jacreuse") || !strcasecmp(name, "iopt10")) {
		int32_t val = 0;
		if (src && (ret = src->_vptr->conv(src, 'i', &val)) < 0) return ret;
		if (val < 0 || val > 1) return MPT_ERROR(BadValue);
		lx->iopt[9] = val;
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "single") || !strcasecmp(name, "iopt12")) {
		int32_t val = 0;
		if (src && (ret = src->_vptr->conv(src, 'i', &val)) < 0) return ret;
		if (val < 0 || val > 1) return MPT_ERROR(BadValue);
		lx->iopt[11] = val;
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "denseout")) {
		return MPT_ERROR(BadOperation);
	}
	if (!strcasecmp(name, "tend") || !strcasecmp(name, "ropt3")) {
		double val;
		if (src && (ret = src->_vptr->conv(src, 'i', &val)) < 0) return ret;
		if (ret) {
			lx->ropt[2] = val;
			lx->iopt[16] = 1;
			return 1;
		} else {
			lx->iopt[16] = 0;
			return 0;
		}
	}
	if (!strcasecmp(name, "plotjac") || !strcasecmp(name, "iopt18")) {
		int32_t val = 0;
		if (src && (ret = src->_vptr->conv(src, 'i', &val)) < 0) return ret;
		if (val < -1) return MPT_ERROR(BadValue);
		lx->iopt[17] = val;
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "maxstep") || !strcasecmp(name, "ropt1")) {
		double val = 0;
		if (src && (ret = src->_vptr->conv(src, 'd', &val)) < 0) return ret;
		if (val < 0) return MPT_ERROR(BadValue);
		lx->ropt[0] = val;
		return ret ? 1 : 0;
	}
	if (!strncasecmp(name, "yprime", 2) || !strcasecmp(name, "ys")) {
		return mpt_vecpar_set(&lx->ys, src ? (lx->ivp.pint + 1) * lx->ivp.neqs : -1, src);
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
		else { id = mpt_vecpar_get(&lx->atol, &prop->val); }
		prop->name = "atol"; prop->desc = "absolute tolerances";
		return id;
	}
	if (name ? !strcasecmp(name, "rtol") : pos == id++) {
		if (!lx) { prop->val.fmt = "d"; prop->val.ptr = &lx->rtol.d.val; }
		else { id = mpt_vecpar_get(&lx->rtol, &prop->val); }
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
