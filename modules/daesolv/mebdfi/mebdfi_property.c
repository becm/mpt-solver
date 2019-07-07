/*!
 * set MEBDFI integrator parameter
 */

#include <string.h>
#include <strings.h>

#include "mebdfi.h"

#include "version.h"
#include "meta.h"

#include "module_functions.h"

static int setInt(MPT_SOLVER_STRUCT(mebdfi) *me, size_t pos, int val)
{
	int *d;
	if (!(d = mpt_solver_module_valloc(&me->iwork, pos + 1, sizeof(int)))) {
		return MPT_ERROR(BadOperation);
	}
	d[pos] = val;
	return 0;
}

static int setJacobian(MPT_SOLVER_STRUCT(mebdfi) *me, MPT_INTERFACE(convertable) *src)
{
	MPT_STRUCT(consumable) val;
	const char *key;
	int32_t ld, ud;
	int ret, jnum;
	long max;
	char mode;
	
	if (!src) {
		me->jnum = me->jbnd = 0;
		return 0;
	}
	key = 0;
	if ((ret = mpt_consumable_setup(&val, src)) < 0) {
		if ((ret = src->_vptr->convert(src, 'k', &key)) < 0) {
			return ret;
		}
		ret = 0;
	}
	else if ((ret = mpt_consume_key(&val, &key)) < 0) {
		return ret;
	} else {
		ret = 1;
	}
	if (!key || (mode = *key)) {
		me->jnum = me->jbnd = 0;
		return ret;
	}
	switch (mode) {
		case 'F': me->jnum = 0; me->jbnd = 0; return ret;
		case 'f': me->jnum = 1; me->jbnd = 0; return ret;
		case 'B': jnum = 0; break;
		case 'b': jnum = 1; break;
		default: return MPT_ERROR(BadValue);
	}
	if ((ret = mpt_consume_int(&val, &ld)) < 0) {
		return ret;
	}
	else if (!ret) {
		ld = ud = me->ivp.neqs;
		ret = 1;
	}
	else if ((ret = mpt_consume_int(&val, &ud)) < 0) {
		return ret;
	}
	else if (!ret) {
		ud = ld;
		ret = 2;
	}
	else {
		ret = 3;
	}
	if (ld < 0 || ud < 0) {
		return MPT_ERROR(BadValue);
	}
	max = me->ivp.neqs * (me->ivp.pint + 1);
	if (ld >= max || ud >= max) {
		return MPT_ERROR(BadValue);
	}
	
	me->jbnd = 1;
	me->jnum = jnum;
	
	me->mbnd[0] = ld;
	me->mbnd[1] = ud;
	me->mbnd[2] =     ld + ud + 1;
	me->mbnd[3] = 2 * ld + ud + 1;
	
	return ret;
}

extern int mpt_mebdfi_set(MPT_SOLVER_STRUCT(mebdfi) *me, const char *name, MPT_INTERFACE(convertable) *src)
{
	long maxval;
	int ret = 0;
	
	/* initial values */
	if (!name) {
		return MPT_SOLVER_MODULE_FCN(ivp_state)(&me->ivp, &me->t, &me->y, src);
	}
	if (!*name) {
		MPT_IVP_STRUCT(parameters) ivp = MPT_IVPPAR_INIT;
		
		if (src && (ret =  mpt_solver_module_ivpset(&ivp, src)) < 0) {
			return ret;
		}
		mpt_mebdfi_fini(me);
		mpt_mebdfi_init(me);
		me->ivp = ivp;
		
		return ret;
	}
	
	if (!strcasecmp(name, "atol")) {
		return mpt_solver_module_tol_set(&me->atol, src, __MPT_IVP_ATOL);
	}
	if (!strcasecmp(name, "rtol")) {
		return mpt_solver_module_tol_set(&me->rtol, src, __MPT_IVP_RTOL);
	}
	if (!strncasecmp(name, "jacobian", 3)) {
		return setJacobian(me, src);
	}
	if (!strcasecmp(name, "h0") || !strcasecmp(name, "stepinit") || !strcasecmp(name, "initstep")) {
		double val = 0;
		if (src && (ret = src->_vptr->convert(src, 'd', &val)) < 0) {
			return ret;
		}
		if (val < 0) {
			return MPT_ERROR(BadValue);
		}
		me->h = val;
		return 0;
	}
	if (!strcasecmp(name, "maxstp") || !strcasecmp(name, "maxstep") || !strcasecmp(name, "iwork14")) {
		int32_t val = 0;
		if (src && (ret = src->_vptr->convert(src, 'i', &val)) < 0) {
			return ret;
		}
		if (val < 0) {
			return MPT_ERROR(BadValue);
		}
		if (setInt(me, 13, val) < 0) {
			return MPT_ERROR(BadOperation);
		}
		return 0;
	}
	maxval = me->ivp.neqs * (me->ivp.pint + 1);
	if (!strcasecmp(name, "nind1")) {
		int32_t val = 0;
		if (src && (ret = src->_vptr->convert(src, 'i', &val)) < 0) {
			return ret;
		}
		if (val < 0 || val > maxval) {
			return MPT_ERROR(BadValue);
		}
		if (setInt(me, 0, val) < 0) {
			return MPT_ERROR(BadOperation);
		}
		return 0;
	}
	if (!strcasecmp(name, "nind2")) {
		int32_t val = 0;
		if (src && (ret = src->_vptr->convert(src, 'i', &val)) < 0) {
			return ret;
		}
		if (val < 0 || val > maxval) {
			return MPT_ERROR(BadValue);
		}
		if (setInt(me, 1, val) < 0) {
			return MPT_ERROR(BadOperation);
		}
		return 0;
	}
	if (!strcasecmp(name, "nind3")) {
		int32_t val = 0;
		if (src && (ret = src->_vptr->convert(src, 'i', &val)) < 0) {
			return ret;
		}
		if (val < 0 || val > maxval) {
			return MPT_ERROR(BadValue);
		}
		if (setInt(me, 2, val) < 0) {
			return MPT_ERROR(BadOperation);
		}
		return ret ? 1 : 0;
	}
	if (!strncasecmp(name, "yp", 2)) {
		return MPT_SOLVER_MODULE_FCN(ivp_vecset)(&me->ivp, &me->yp, src);
	}
	return MPT_ERROR(BadArgument);
}

extern int mpt_mebdfi_get(const MPT_SOLVER_STRUCT(mebdfi) *me, MPT_STRUCT(property) *prop)
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
		prop->name = "mebdfi";
		prop->desc = "implicit DAE solver with BDF";
		return mpt_solver_module_value_ivp(&prop->val, &me->ivp);
	}
	else if (!strcasecmp(name, "version")) {
		static const char version[] = BUILD_VERSION"\0";
		prop->name = "version"; prop->desc = "solver release information";
		prop->val.fmt= 0; prop->val.ptr = version;
		return 0;
	}
	
	id = -1;
	if (name ? !strcasecmp(name, "atol") : pos == ++id) {
		prop->name = "atol";
		prop->desc = "absolute tolerances";
		if (me) {
			return mpt_solver_module_tol_get(&prop->val, &me->atol);
		}
		mpt_solver_module_value_double(&prop->val, &me->atol._d.val);
		return id;
	}
	if (name ? !strcasecmp(name, "rtol") : pos == ++id) {
		prop->name = "rtol";
		prop->desc = "relative tolerances";
		if (me) {
			return mpt_solver_module_tol_get(&prop->val, &me->rtol);
		}
		mpt_solver_module_value_double(&prop->val, &me->rtol._d.val);
		return id;
	}
	if (name ? !strncasecmp(name, "jac", 3) : pos == ++id) {
		static const uint8_t fmt[] = "ii";
		prop->name = "jacobian";
		prop->desc = "(user) jacobian parameters";
		prop->val.fmt = fmt;
		prop->val.ptr = me->mbnd;
		if (!me) return id;
		return me->jbnd ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "h0") || !strcasecmp(name, "stepinit") || !strcasecmp(name, "initstep")) : pos == ++id) {
		prop->name = "stepinit";
		prop->desc = "explicit initial step size";
		mpt_solver_module_value_double(&prop->val, &me->h);
		if (!me) return id;
		return me->h ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "maxstp") || !strcasecmp(name, "maxstep") || !strcasecmp(name, "iwork14")) : pos == ++id) {
		prop->name = "maxstep";
		prop->desc = "max. internal steps per call";
		pos = mpt_solver_module_value_ivec(&prop->val, 14, me ? &me->iwork : 0);
		return me ? pos : id;
	}
	if (name ? !strcasecmp(name, "nind1") : pos == ++id) {
		prop->name = "nind1";
		prop->desc = "index1 variables";
		pos = mpt_solver_module_value_ivec(&prop->val, 1, me ? &me->iwork : 0);
		return me ? pos : id;
	}
	if (name ? !strcasecmp(name, "nind2") : pos == ++id) {
		prop->name = "nind2";
		prop->desc = "index2 variables";
		pos = mpt_solver_module_value_ivec(&prop->val, 2, me ? &me->iwork : 0);
		return me ? pos : id;
	}
	if (name ? !strcasecmp(name, "nind3") : pos == ++id) {
		prop->name = "nind3";
		prop->desc = "index3 variables";
		pos = mpt_solver_module_value_ivec(&prop->val, 3, me ? &me->iwork : 0);
		return me ? pos : id;
	}
	/* state properties */
	if (!name || !me) {
		return MPT_ERROR(BadArgument);
	}
	if (!strncasecmp(name, "yp", 2)) {
		prop->name = "yp";
		prop->desc = "current deviation vector";
		mpt_solver_module_value_double(&prop->val, me->y);
		pos = me->ivp.pint + 1;
		return me->ivp.neqs * pos;
	}
	return MPT_ERROR(BadArgument);
}
