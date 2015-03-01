/*!
 * set radau integrator parameter.
 */

#include <errno.h>
#include <string.h>
#include <strings.h>

#include "release.h"
#include "version.h"

#include "radau.h"

static int setIvp(MPT_SOLVER_STRUCT(radau) *data, MPT_INTERFACE(source) *src)
{
	if (!src) {
		return mpt_ivppar_set(&data->ivp, src);
	}
	else {
		MPT_SOLVER_STRUCT(ivppar) ivp = data->ivp;
		int ret;
		
		if ((ret =  mpt_ivppar_set(&ivp, src)) < 0) {
			return ret;
		}
		mpt_radau_fini(data);
		if (mpt_radau_init(data) < 0) {
			return -1;
		}
		data->ivp = ivp;
		return ret;
	}
}
static int setJacobian(MPT_SOLVER_STRUCT(radau) *data, MPT_INTERFACE(source) *src)
{
	int l1, l2, l3;
	char *key;
	
	if (!src) return data->ijac;
	
	if ((l1 = src->_vptr->conv(src, 'k', &key)) < 0) return l1;
	
	if (!l1) return data->ijac = 0;
	
	l2 = l3 = 0;
	
	switch (key[0]) {
		case 'f':
			data->ijac = 0;
		case 'F':
			l2 = data->ivp.neqs * (data->ivp.pint + 1);
			data->mljac = data->mujac = l2;
			l2 = 0;
			break;
		case 'b':
			data->ijac = 0;
		case 'B':
			if ((l2 = src->_vptr->conv(src, 'i', &data->mljac)) <= 0) {
				l2 = 0; data->mljac = data->mujac = data->ivp.neqs;
			}
			else if ((l3 = src->_vptr->conv(src, 'i', &data->mujac)) <= 0) {
				l3 = 0; data->mujac = data->mljac;
			}
			break;
		default:
			errno = EINVAL; return -2;
	}
	if (data->mlmas > data->mljac) data->mlmas = data->mljac;
	if (data->mumas > data->mujac) data->mumas = data->mujac;
	
	return l1 + l2 + l3;
}
static int setInitStep(MPT_SOLVER_STRUCT(radau) *data, MPT_INTERFACE(source) *src)
{
	int len;
	
	if (!src) return data->h ? 1 : 0;
	
	if ((len = src->_vptr->conv(src, 'd', &data->h)) < 0) return -2;
	if (!len) data->h = 0.0;
	return len;
}

extern int mpt_radau_property(MPT_SOLVER_STRUCT(radau) *data, MPT_STRUCT(property) *prop, MPT_INTERFACE(source) *src)
{
	const char *name;
	intptr_t pos = -1, id;
	
	if (!prop) return (src && data) ? setIvp(data, src) : MPT_ENUM(TypeSolver);
	
	if (!(name = prop->name)) {
		if (src || ((pos = (intptr_t) prop->desc) < 0)) {
			errno = EINVAL;
			return -3;
		}
	}
	else if (!*name) {
		id = MPT_SOLVER_ENUM(ODE) | MPT_SOLVER_ENUM(DAE) | MPT_SOLVER_ENUM(PDE);
		if (data && src && (id = setIvp(data, src)) < 0) return id;
		prop->name = "radau"; prop->desc = "implicit Runge-Kutta DAE solver";
		prop->fmt  = "iid"; prop->data = &data->ivp;
		return id;
	}
	if (name && !strcasecmp(name, "version")) {
		static const char version[] = MPT_VERSION"\0";
		prop->name = "version"; prop->desc = "solver release information";
		prop->fmt = 0; prop->data = version;
		return 0;
	}
	
	id = 0;
	if (name ? !strcasecmp(name, "atol") : pos == id++) {
		if (data && (id = mpt_vecpar_property(&data->atol, prop, src)) < 0) return id;
		prop->name = "atol"; prop->desc = "absolute tolerances";
		return id;
	}
	if (name ? !strcasecmp(name, "rtol") : pos == id++) {
		if (data && (id = mpt_vecpar_property(&data->rtol, prop, src)) < 0) return id;
		prop->name = "rtol"; prop->desc = "relative tolerances";
		return id;
	}
	if (name ? !strncasecmp(name, "jac", 3) : pos == id++) {
		if (data && (id = setJacobian(data, src)) < 0) return id;
		prop->name = "jacobian"; prop->desc = "(user) jacobian parameters";
		prop->fmt  = "i"; prop->data = &data->ijac;
		return id;
	}
	/* set initial stepsize */
	if (name ? !strcasecmp(name, "stepinit") : pos == id++) {
		if (data && (id = setInitStep(data, src)) < 0) return id;
		prop->name = "stepinit"; prop->desc = "explicit initial stepsize";
		prop->fmt = "d"; prop->data = &data->h;
		return id;
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
	
	return -1;
}
