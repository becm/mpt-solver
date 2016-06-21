/*!
 * set MEBDFI integrator parameter
 */

#include <string.h>
#include <strings.h>
#include <errno.h>

#include "version.h"

#include "mebdfi.h"

static int setInt(MPT_SOLVER_STRUCT(mebdfi) *me, size_t pos, int val)
{
	int *d;
	if (!(d = mpt_vecpar_alloc(&me->iwork, pos+1, sizeof(int)))) {
		return MPT_ERROR(BadOperation);
	}
	d[pos] = val;
	return 0;
}

static int setJacobian(MPT_SOLVER_STRUCT(mebdfi) *me, MPT_INTERFACE(metatype) *src)
{
	char *key;
	int32_t ld, ud;
	int ret, len, jnum;
	
	if (!src) {
		me->jnum = me->jbnd = 0;
		return 0;
	}
	if ((ret = src->_vptr->conv(src, 'k' | MPT_ENUM(ValueConsume), &key)) < 0) {
		return ret;
	}
	if (!ret || !key) {
		me->jnum = me->jbnd = 0;
		return 0;
	}
	switch (key[0]) {
		case 'F': me->jnum = 0; me->jbnd = 0; return 1;
		case 'f': me->jnum = 1; me->jbnd = 0; return 1;
		case 'B': jnum = 0; break;
		case 'b': jnum = 1; break;
		default: return MPT_ERROR(BadValue);
	}
	if ((ret = src->_vptr->conv(src, 'i' | MPT_ENUM(ValueConsume), &ld)) < 0) {
		return ret;
	}
	if (!ld) {
		ld = me->ivp.neqs;
		ud = ld;
		len = 1;
	}
	else if ((ret = src->_vptr->conv(src, 'i' | MPT_ENUM(ValueConsume), &ud)) < 0) {
		return ret;
	}
	else if (!ret) {
		ud = ld;
		len = 2;
	}
	else {
		len = 3;
	}
	ret = me->ivp.neqs * (me->ivp.pint + 1);
	if (ld < 0 || ld > ret
	    || ud < 0 || ud > ret) {
		return MPT_ERROR(BadValue);
	}
	
	me->jbnd = 1;
	me->jnum = jnum;
	
	me->mbnd[0] = ld;
	me->mbnd[1] = ud;
	me->mbnd[2] =   ld + ud + 1;
	me->mbnd[3] = 2*ld + ud + 1;
	
	return len;
}

extern int mpt_mebdfi_set(MPT_SOLVER_STRUCT(mebdfi) *me, const char *name, MPT_INTERFACE(metatype) *src)
{
	int maxval;
	int ret = 0;
	
	/* initial values */
	if (!name) {
		double t = me->t;
		int required;
		
		if (src && (ret = src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), &t) <= 0)) {
			if (ret < 0) {
				return ret;
			}
			src = 0;
		}
		required = me->ivp.neqs * (me->ivp.pint + 1);
		if ((ret = mpt_vecpar_set(&me->y, required, src)) < 0) {
			return ret;
		}
		me->t = t;
		
		return src ? ++ret : 0;
	}
	/* change solver dimensions, reinit */
	if (!*name) {
		MPT_SOLVER_IVP_STRUCT(parameters) ivp = me->ivp;
		
		if (src && (ret =  mpt_ivppar_set(&ivp, src)) < 0) {
			return ret;
		}
		mpt_mebdfi_fini(me);
		mpt_mebdfi_init(me);
		
		me->ivp = ivp;
		return ret;
	}
	
	if (!strcasecmp(name, "atol")) {
		return mpt_vecpar_settol(&me->atol, src, __MPT_IVP_ATOL);
	}
	if (!strcasecmp(name, "rtol")) {
		return mpt_vecpar_settol(&me->rtol, src, __MPT_IVP_RTOL);
	}
	if (!strncasecmp(name, "jacobian", 3)) {
		return setJacobian(me, src);
	}
	if (!strcasecmp(name, "h0") || !strcasecmp(name, "stepinit") || !strcasecmp(name, "initstep")) {
		double val = 0;
		if (src && (ret = src->_vptr->conv(src, 'd', &val)) < 0) return ret;
		if (val < 0) return MPT_ERROR(BadValue);
		me->h = val;
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "maxstp") || !strcasecmp(name, "maxstep") || !strcasecmp(name, "iwork14")) {
		int32_t val = 0;
		if (src && (ret = src->_vptr->conv(src, 'i', &val)) < 0) return ret;
		if (val < 0) return MPT_ERROR(BadValue);
		if (setInt(me, 13, val) < 0) return MPT_ERROR(BadOperation);
		return ret ? 1 : 0;
	}
	maxval = me->ivp.neqs * (me->ivp.pint + 1);
	if (!strcasecmp(name, "nind1")) {
		int32_t val = 0;
		if (src && (ret = src->_vptr->conv(src, 'i', &val)) < 0) return ret;
		if (val < 0 || val > maxval) return MPT_ERROR(BadValue);
		if (setInt(me, 0, val) < 0) return MPT_ERROR(BadOperation);
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "nind2")) {
		int32_t val = 0;
		if (src && (ret = src->_vptr->conv(src, 'i', &val)) < 0) return ret;
		if (val < 0 || val > maxval) return MPT_ERROR(BadValue);
		if (setInt(me, 1, val) < 0) return MPT_ERROR(BadOperation);
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "nind3")) {
		int32_t val = 0;
		if (src && (ret = src->_vptr->conv(src, 'i', &val)) < 0) return ret;
		if (val < 0 || val > maxval) return MPT_ERROR(BadValue);
		if (setInt(me, 2, val) < 0) return MPT_ERROR(BadOperation);
		return ret ? 1 : 0;
	}
	if (!strncasecmp(name, "yp", 2)) {
		return mpt_vecpar_set(&me->y, maxval, src);
	}
	return MPT_ERROR(BadArgument);
}

extern int mpt_mebdfi_get(const MPT_SOLVER_STRUCT(mebdfi) *me, MPT_STRUCT(property) *prop)
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
		prop->name = "mebdfi"; prop->desc = "implicit DAE solver with BDF";
		prop->val.fmt = "iid"; prop->val.ptr = &me->ivp;
		return MPT_SOLVER_ENUM(ODE) | MPT_SOLVER_ENUM(DAE) | MPT_SOLVER_ENUM(PDE);
	}
	else if (!strcasecmp(name, "version")) {
		static const char version[] = BUILD_VERSION"\0";
		prop->name = "version"; prop->desc = "solver release information";
		prop->val.fmt= 0; prop->val.ptr = version;
		return 0;
	}
	
	id = -1;
	if (name ? !strcasecmp(name, "atol") : pos == ++id) {
		if (!me) { prop->val.fmt = "d"; prop->val.ptr = &me->atol.d.val; }
		else { id = mpt_vecpar_get(&me->atol, &prop->val); }
		prop->name = "atol"; prop->desc = "absolute tolerances";
		return id;
	}
	if (name ? !strcasecmp(name, "rtol") : pos == ++id) {
		if (!me) { prop->val.fmt = "d"; prop->val.ptr = &me->rtol.d.val; }
		else { id = mpt_vecpar_get(&me->rtol, &prop->val); }
		prop->name = "rtol"; prop->desc = "relative tolerances";
		return id;
	}
	if (name ? !strncasecmp(name, "jac", 3) : pos == ++id) {
		prop->name = "jacobian"; prop->desc = "(user) jacobian parameters";
		prop->val.fmt = "ii"; prop->val.ptr = me->mbnd;
		if (!me) return id;
		return me->jbnd ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "h0") || !strcasecmp(name, "stepinit") || !strcasecmp(name, "initstep")) : pos == ++id) {
		prop->name = "stepinit"; prop->desc = "explicit initial step size";
		prop->val.fmt = "d"; prop->val.ptr = &me->h;
		if (!me) return id;
		return me->h ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "maxstp") || !strcasecmp(name, "maxstep") || !strcasecmp(name, "iwork14")) : pos == ++id) {
		int *v;
		prop->name = "maxstep"; prop->desc = "max. internal steps per call";
		prop->val.fmt = "i"; prop->val.ptr = 0;
		if (!me) return id;
		if (me->iwork.iov_len / sizeof(int) <= 13) return 0;
		prop->val.ptr = v = ((int *) me->iwork.iov_base);
		return v[13] ? 1 : 0;
	}
	if (name ? !strcasecmp(name, "nind1") : pos == ++id) {
		int *v;
		prop->name = "nind1"; prop->desc = "index1 variables";
		prop->val.fmt = "i"; prop->val.ptr = 0;
		if (!me) return id;
		if (me->iwork.iov_len / sizeof(int) <= 0) return 0;
		prop->val.ptr = v = ((int *) me->iwork.iov_base);
		return v[0] ? 1 : 0;
	}
	if (name ? !strcasecmp(name, "nind2") : pos == ++id) {
		int *v;
		prop->name = "nind1"; prop->desc = "index1 variables";
		prop->val.fmt = "i"; prop->val.ptr = 0;
		if (!me) return id;
		if (me->iwork.iov_len / sizeof(int) <= 1) return 0;
		prop->val.ptr = v = ((int *) me->iwork.iov_base);
		return v[1] ? 1 : 0;
	}
	if (name ? !strcasecmp(name, "nind3") : pos == ++id) {
		int *v;
		prop->name = "nind1"; prop->desc = "index1 variables";
		prop->val.fmt = "i"; prop->val.ptr = 0;
		if (!me) return id;
		if (me->iwork.iov_len / sizeof(int) <= 2) return 0;
		prop->val.ptr = v = ((int *) me->iwork.iov_base);
		return v[2] ? 1 : 0;
	}
	if (name ? !strncasecmp(name, "yp", 2) : pos == ++id) {
		prop->name = "yp"; prop->desc = "max. internal steps per call";
		prop->val.fmt = "d"; prop->val.ptr = 0;
		if (!me) return id;
		if (!(prop->val.ptr = me->y)) return 0;
		return me->ivp.neqs * (me->ivp.pint + 1);
	}
	return MPT_ERROR(BadArgument);
}
