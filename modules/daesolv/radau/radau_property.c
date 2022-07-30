/*!
 * set radau integrator parameter.
 */

#include <string.h>
#include <strings.h>

#include "radau.h"

#include "version.h"
#include "types.h"
#include "meta.h"

#include "module_functions.h"

static int setJacobian(MPT_SOLVER_STRUCT(radau) *rd, MPT_INTERFACE(convertable) *src)
{
	MPT_INTERFACE(iterator) *it;
	const char *key;
	int32_t ld, ud;
	long len;
	int ret;
	char mode;
	
	if (!src) {
		rd->ijac = 0;
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
		rd->ijac = 0;
		return ret;
	}
	len = rd->ivp.neqs * (rd->ivp.pint + 1);
	ld = len;
	ud = ld;
	
	switch (mode) {
		case 'f':
			rd->ijac = 0;
			/* fall through */
		case 'F':
			break;
		case 'b':
			rd->ijac = 0;
			/* fall through */
		case 'B':
			if (!it || (ret = mpt_solver_module_consume_value(it, 'i', &ld, sizeof(ld))) < 0) {
				ld = rd->ivp.neqs;
				if (!rd->ivp.pint) {
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
			len = rd->ivp.neqs * (rd->ivp.pint + 1);
			if (ld > len || ud > len) {
				return MPT_ERROR(BadValue);
			}
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

extern int mpt_radau_set(MPT_SOLVER_STRUCT(radau) *rd, const char *name, MPT_INTERFACE(convertable) *src)
{
	int ret = 0;
	
	/* initial values */
	if (!name) {
		return MPT_SOLVER_MODULE_FCN(ivp_state)(&rd->ivp, &rd->t, &rd->y, src);
	}
	if (!*name) {
		MPT_IVP_STRUCT(parameters) ivp = MPT_IVPPAR_INIT;
		
		if (src && (ret =  mpt_solver_module_ivpset(&ivp, src)) < 0) {
			return ret;
		}
		mpt_radau_fini(rd);
		mpt_radau_init(rd);
		rd->ivp = ivp;
		
		return ret;
	}
	if (!strcasecmp(name, "atol")) {
		return mpt_solver_module_tol_set(&rd->atol, src, __MPT_IVP_ATOL);
	}
	if (!strcasecmp(name, "rtol")) {
		return mpt_solver_module_tol_set(&rd->rtol, src, __MPT_IVP_RTOL);
	}
	if (!strncasecmp(name, "jacobian", 3)) {
		return setJacobian(rd, src);
	}
	/* set initial stepsize */
	if (!strcasecmp(name, "stepinit") || !strcasecmp(name, "initstep")) {
		double val = 0;
		if (src && (ret = src->_vptr->convert(src, 'd', &val)) < 0) {
			return ret;
		}
		if (val < 0) {
			return MPT_ERROR(BadValue);
		}
		rd->h = val;
		return 0;
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
		return MPT_SOLVER_ENUM(DAE) | MPT_SOLVER_ENUM(PDE);
	}
	if (!(name = prop->name)) {
		if ((pos = (intptr_t) prop->desc) < 0) {
			return MPT_ERROR(BadArgument);
		}
	}
	else if (!*name) {
		prop->name = "radau";
		prop->desc = "implicit Runge-Kutta DAE solver";
		mpt_solver_module_value_ivp(prop, rd ? &rd->ivp : 0);
		return (rd->ivp.neqs == 1 && !rd->ivp.pint) ? 0 : 1;
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
		if (!rd) {
			MPT_value_set(&prop->val, 'd', 0);
			return id;
		}
		return mpt_solver_module_tol_get(prop, &rd->atol);
	}
	if (name ? !strcasecmp(name, "rtol") : pos == ++id) {
		prop->name = "rtol";
		prop->desc = "relative tolerances";
		if (!rd) {
			MPT_value_set(&prop->val, 'd', 0);
			return id;
		}
		return mpt_solver_module_tol_get(prop, &rd->rtol);
	}
	if (name ? !strncasecmp(name, "jacobian", 3) : pos == ++id) {
		prop->name = "jacobian";
		prop->desc = "(user) jacobian parameters";
		if (!rd) {
			MPT_value_set(&prop->val, 'i', 0);
			return id;
		}
		return mpt_solver_module_value_int(prop, &rd->ijac);
	}
	/* set initial stepsize */
	if (name ? (!strcasecmp(name, "stepinit") || !strcasecmp(name, "initstep")) : pos == ++id) {
		prop->name = "stepinit";
		prop->desc = "explicit initial stepsize";
		if (!rd) {
			MPT_value_set(&prop->val, 'd', 0);
			return id;
		}
		return mpt_solver_module_value_double(prop, &rd->h);
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
