/*!
 * set radau integrator parameter.
 */

#include <errno.h>
#include <string.h>
#include <strings.h>

#include "version.h"

#include "radau.h"

static int setJacobian(MPT_SOLVER_STRUCT(radau) *rd, MPT_INTERFACE(metatype) *src)
{
	char *key;
	int32_t ld, ud;
	int ret, len;
	
	if (!src) {
		rd->ijac = 0;
		return 0;
	}
	if ((ret = src->_vptr->conv(src, 'k' | MPT_ENUM(ValueConsume), &key)) < 0) {
		return ret;
	}
	if (!ret || !key) {
		rd->ijac = 0;
		return 0;
	}
	len = rd->ivp.neqs * (rd->ivp.pint + 1);
	ld = len;
	ud = ld;
	
	switch (key[0]) {
		case 'f':
			rd->ijac = 0;
		case 'F':
			break;
		case 'b':
			rd->ijac = 0;
		case 'B':
			if ((ret = src->_vptr->conv(src, 'i' | MPT_ENUM(ValueConsume), &ld)) < 0) {
				return ret;
			}
			if (!ret) {
				ld = rd->ivp.neqs;
				ud = ld;
				break;
			}
			if (ld < 0 || ld > len) {
				return MPT_ERROR(BadValue);
			}
			if ((ret = src->_vptr->conv(src, 'i' | MPT_ENUM(ValueConsume), &ud)) < 0) {
				return ret;
			}
			if (!ret) {
				len = 2;
				break;
			}
			if (ud < 0 || ud > len) {
				return MPT_ERROR(BadValue);
			}
			len = 3;
			break;
		default:
			return MPT_ERROR(BadValue);
	}
	rd->mljac = ld;
	rd->mujac = ud;
	if (rd->mlmas > ld) rd->mlmas = ld;
	if (rd->mumas > ud) rd->mumas = ud;
	
	return len;
}

extern int mpt_radau_set(MPT_SOLVER_STRUCT(radau) *rd, const char *name, MPT_INTERFACE(metatype) *src)
{
	int ret = 0;
	
	/* initial values */
	if (!name) {
		double t = rd->t;
		size_t required;
		
		if (src && (ret = src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), &t) <= 0)) {
			if (ret < 0) {
				return ret;
			}
			src = 0;
		}
		required = rd->ivp.neqs * (rd->ivp.pint + 1);
		if ((ret = mpt_vecpar_set(&rd->y, required, src)) < 0) {
			return ret;
		}
		rd->t = t;
		
		return src ? ++ret : 0;
	}
	/* change solver dimensions, reinit */
	if (!*name) {
		MPT_SOLVER_STRUCT(ivppar) ivp = rd->ivp;
		
		if (src && (ret =  mpt_ivppar_set(&ivp, src)) < 0) {
			return ret;
		}
		mpt_radau_fini(rd);
		mpt_radau_init(rd);
		
		rd->ivp = ivp;
		return ret;
	}
	if (!strcasecmp(name, "atol")) {
		return mpt_vecpar_settol(&rd->atol, src, __MPT_IVP_ATOL);
	}
	if (!strcasecmp(name, "rtol")) {
		return mpt_vecpar_settol(&rd->rtol, src, __MPT_IVP_RTOL);
	}
	if (!strncasecmp(name, "jacobian", 3)) {
		return setJacobian(rd, src);
	}
	/* set initial stepsize */
	if (!strcasecmp(name, "stepinit") || !strcasecmp(name, "initstep")) {
		double val = 0;
		if (src && (ret = src->_vptr->conv(src, 'd', &val)) < 0) return ret;
		if (val < 0) return MPT_ERROR(BadValue);
		rd->h = val;
		return ret ? 1 : 0;
	}
	/* set integer parameter
	if ( !strncasecmp(param, "iwork", 5) ) {
		char	*end;
		long	val = strtol(param+=5, &end, 0);
		
		if ( end <= param ) return -2;
		
		if ( val < 1 || val > 13 ) return -3;
		
		return fcn(data->iwork + (val - 1), 'i', fpar);
	}
	set double parameter
	if ( !strncasecmp(param, "work", 4) ) {
		char	*end;
		long	val = strtol(param+=4, &end, 0);
		
		if ( end <= param ) return -2;
		
		if ( val < 1 || val > 13 ) return -3;
		
		return fcn(data->iwork + (val - 1), 'd', fpar);
	} */
	return MPT_ERROR(BadArgument);
}

extern int mpt_radau_get(const MPT_SOLVER_STRUCT(radau) *rd, MPT_STRUCT(property) *prop)
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
		prop->name = "radau"; prop->desc = "implicit Runge-Kutta DAE solver";
		prop->val.fmt  = "ii"; prop->val.ptr = &rd->ivp;
		return MPT_SOLVER_ENUM(ODE) | MPT_SOLVER_ENUM(DAE) | MPT_SOLVER_ENUM(PDE);
	}
	else if (!strcasecmp(name, "version")) {
		static const char version[] = BUILD_VERSION"\0";
		prop->name = "version"; prop->desc = "solver release information";
		prop->val.fmt = 0; prop->val.ptr = version;
		return 0;
	}
	
	id = -1;
	if (name ? !strcasecmp(name, "atol") : pos == ++id) {
		if (!rd) { prop->val.fmt = "d"; prop->val.ptr = &rd->atol.d.val; }
		else { id = mpt_vecpar_get(&rd->atol, &prop->val); }
		prop->name = "atol"; prop->desc = "absolute tolerances";
		return id;
	}
	if (name ? !strcasecmp(name, "rtol") : pos == ++id) {
		if (!rd) { prop->val.fmt = "d"; prop->val.ptr = &rd->rtol.d.val; }
		else { id = mpt_vecpar_get(&rd->rtol, &prop->val); }
		prop->name = "rtol"; prop->desc = "relative tolerances";
		return id;
	}
	if (name ? !strncasecmp(name, "jacobian", 3) : pos == ++id) {
		prop->name = "jacobian"; prop->desc = "(user) jacobian parameters";
		prop->val.fmt  = "i"; prop->val.ptr = &rd->ijac;
		return id;
	}
	/* set initial stepsize */
	if (name ? (!strcasecmp(name, "stepinit") || !strcasecmp(name, "initstep")) : pos == ++id) {
		prop->name = "stepinit"; prop->desc = "explicit initial stepsize";
		prop->val.fmt = "d"; prop->val.ptr = &rd->h;
		if (!rd) return id;
		return rd->h ? 1 : 0;
	}
	/* set integer parameter
	if ( !strncasecmp(param, "iwork", 5) ) {
		char	*end;
		long	val = strtol(param+=5, &end, 0);
		
		if ( end <= param ) return -2;
		
		if ( val < 1 || val > 13 ) return -3;
		
		return fcn(data->iwork + (val - 1), 'i', fpar);
	}
	set double parameter
	if ( !strncasecmp(param, "work", 4) ) {
		char	*end;
		long	val = strtol(param+=4, &end, 0);
		
		if ( end <= param ) return -2;
		
		if ( val < 1 || val > 13 ) return -3;
		
		return fcn(data->iwork + (val - 1), 'd', fpar);
	} */
	
	errno = EINVAL;
	return MPT_ERROR(BadArgument);
}
