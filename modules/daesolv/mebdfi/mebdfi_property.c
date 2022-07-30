/*!
 * set MEBDFI integrator parameter
 */

#include <string.h>
#include <strings.h>

#include "mebdfi.h"

#include "version.h"
#include "types.h"
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
	MPT_INTERFACE(iterator) *it;
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
	it = 0;
	if ((ret = src->_vptr->convert(src, MPT_ENUM(TypeIteratorPtr), &it)) >= 0) {
		ret = mpt_solver_module_consume_value(it, 'k', &key, 0);
	}
	if (ret < 0 && (ret = src->_vptr->convert(src, 'k', &key)) < 0) {
		return ret;
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
	if (!it || (ret = mpt_solver_module_consume_value(it, 'i', &ld, sizeof(ld))) < 0) {
		ld = me->ivp.neqs;
		if (!me->ivp.pint) {
			--ld;
		}
		ud = ld;
	}
	else if (!ret) {
		ud = ld;
		ret = 2;
	}
	else if ((ret = mpt_solver_module_consume_value(it, 'i', &ud, sizeof(ud))) < 0) {
		return ret;
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
		return mpt_solver_module_value_ivp(prop, me ? &me->ivp : 0);
	}
	else if (!strcasecmp(name, "version")) {
		static const char version[] = BUILD_VERSION"\0";
		prop->name = "version";
		prop->desc = "solver release information";
		mpt_solver_module_value_string(prop, version);
		return 0;
	}
	
	id = -1;
	if (name ? !strcasecmp(name, "atol") : pos == ++id) {
		prop->name = "atol";
		prop->desc = "absolute tolerances";
		if (!me) {
			MPT_value_set(&prop->val, 'd', 0);
			return id;
		}
		return mpt_solver_module_tol_get(prop, &me->atol);
	}
	if (name ? !strcasecmp(name, "rtol") : pos == ++id) {
		prop->name = "rtol";
		prop->desc = "relative tolerances";
		if (!me) {
			MPT_value_set(&prop->val, 'd', 0);
			return id;
		}
		return mpt_solver_module_tol_get(prop, &me->rtol);
	}
	if (name ? !strncasecmp(name, "jac", 3) : pos == ++id) {
		const char *type;
		prop->name = "jacobian";
		prop->desc = "(user) jacobian parameters";
		if (!me) {
			MPT_value_set(&prop->val, 's', 0);
			return id;
		}
		if (me->jbnd) {
			type = me->jac ? "Banded" : "banded";
		} else {
			type = me->jac ? "Full" : "full";
		}
		mpt_solver_module_value_string(prop, type);
		return me->jbnd ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "h0") || !strcasecmp(name, "stepinit") || !strcasecmp(name, "initstep")) : pos == ++id) {
		prop->name = "stepinit";
		prop->desc = "explicit initial step size";
		if (!me) {
			MPT_value_set(&prop->val, 'd', 0);
			return id;
		}
		return mpt_solver_module_value_double(prop, &me->h);
	}
	if (name ? (!strcasecmp(name, "maxstp") || !strcasecmp(name, "maxstep") || !strcasecmp(name, "iwork14")) : pos == ++id) {
		prop->name = "maxstep";
		prop->desc = "max. internal steps per call";
		if (!me) {
			MPT_value_set(&prop->val, 'i', 0);
			return id;
		}
		pos = mpt_solver_module_value_ivec(prop, 14, &me->iwork);
		return me ? pos : id;
	}
	if (name ? !strcasecmp(name, "nind1") : pos == ++id) {
		prop->name = "nind1";
		prop->desc = "index1 variables";
		if (!me) {
			MPT_value_set(&prop->val, 'i', 0);
			return id;
		}
		pos = mpt_solver_module_value_ivec(prop, 1, &me->iwork);
		return me ? pos : id;
	}
	if (name ? !strcasecmp(name, "nind2") : pos == ++id) {
		prop->name = "nind2";
		prop->desc = "index2 variables";
		if (!me) {
			MPT_value_set(&prop->val, 'i', 0);
			return id;
		}
		pos = mpt_solver_module_value_ivec(prop, 2, &me->iwork);
		return me ? pos : id;
	}
	if (name ? !strcasecmp(name, "nind3") : pos == ++id) {
		prop->name = "nind3";
		prop->desc = "index3 variables";
		if (!me) {
			MPT_value_set(&prop->val, 'i', 0);
			return id;
		}
		pos = mpt_solver_module_value_ivec(prop, 3, &me->iwork);
		return me ? pos : id;
	}
	/* state properties */
	if (!name || !me) {
		return MPT_ERROR(BadArgument);
	}
	if (!strncasecmp(name, "yp", 2)) {
		const int type = MPT_type_toVector('d');
		prop->name = "yp";
		prop->desc = "current deviation vector";
		if (!me) {
			MPT_value_set(&prop->val, type, 0);
			return id;
		}
		else {
			struct iovec vec;
			vec.iov_base = me->yp;
			vec.iov_len  = me->ivp.neqs * (me->ivp.pint + 1) * sizeof(*me->yp);
			if ((id = mpt_solver_module_value_set(prop, type, &vec, sizeof(vec))) < 0) {
				return id;
			}
			return me->yp ? 1 : 0;
		}
	}
	return MPT_ERROR(BadArgument);
}
