/*!
 * set IDA integrator parameter
 */

#include <string.h>
#include <errno.h>

#include <ida/ida_impl.h>

#include "release.h"
#include "version.h"

#include "sundials.h"

static int setIvp(MPT_SOLVER_STRUCT(ida) *data, MPT_INTERFACE(source) *src)
{
	int ret;
	if ((ret = mpt_ivppar_set(&data->ivp, src)) >= 0 && src) {
		sundials_ida_reset(data);
	}
	return ret;
}
static int setMaxOrd(MPT_SOLVER_STRUCT(ida) *ida, MPT_INTERFACE(source) *src)
{
	int64_t val;
	int len;
	if (!src) return 0;
	if ((len = src->_vptr->conv(src, 'l', &val)) < 0) return len;
	if (IDASetMaxOrd(ida->mem, len ? val : 0) < 0) return MPT_ERROR(BadValue);
	return len;
}
static int setMaxNSteps(MPT_SOLVER_STRUCT(ida) *ida, MPT_INTERFACE(source) *src)
{
	int64_t val;
	int len;
	if (!src) return 0;
	if ((len = src->_vptr->conv(src, 'l', &val)) < 0) return len;
	if (IDASetMaxNumSteps(ida->mem, len ? val : 0) < 0) return MPT_ERROR(BadValue);
	return len;
}
static int setInitStep(MPT_SOLVER_STRUCT(ida) *ida, MPT_INTERFACE(source) *src)
{
	double val;
	int len;
	if (!src) return 0;
	if ((len = src->_vptr->conv(src, 'd', &val)) < 0) return len;
	if (IDASetInitStep(ida->mem, len ? val : 0) < 0) return MPT_ERROR(BadValue);
	return len;
}
static int setMaxStep(MPT_SOLVER_STRUCT(ida) *ida, MPT_INTERFACE(source) *src)
{
	double val;
	int len;
	if (!src) return 0;
	if ((len = src->_vptr->conv(src, 'd', &val)) < 0) return len;
	if (IDASetMaxStep(ida->mem, len ? val : 0) < 0) return MPT_ERROR(BadValue);
	return len;
}
static int setYP(MPT_SOLVER_STRUCT(ida) *ida, MPT_INTERFACE(source) *src)
{
	double *yp;
	int len;
	
	if (!src) return ida->yp ? 1 : 0;
	
	if ((len = ida->ivp.neqs * (ida->ivp.pint + 1)) < 0) {
		errno = EOVERFLOW;
		return -2;
	}
	if (!ida->sd.y && !(ida->sd.y = sundials_nvector_empty(len))) {
		return MPT_ERROR(BadOperation);
	}
	if (!ida->yp && !(ida->yp = N_VClone(ida->sd.y))) {
		return MPT_ERROR(BadOperation);
	}
	yp = N_VGetArrayPointer(ida->yp);
	
	while (src->_vptr->conv(src, 'd', yp) > 0) {
		yp++;
		if (!--len) {
			return 0;
		}
	}
	while (len--) {
		*(yp++) = 0.0;
	}
	return 1;
}

/*!
 * \ingroup mptSundialsIda
 * \brief set IDA property
 * 
 * Query property of IDA solver
 * 
 * \param ida  IDA data
 * \param prop metatype property
 * \param src  data source to change property
 * 
 * \retval 0   default value
 * \retval <0  failure
 * \retval >0  changed property
 */
extern int sundials_ida_property(MPT_SOLVER_STRUCT(ida) *ida, MPT_STRUCT(property) *prop, MPT_INTERFACE(source) *src)
{
	const char *name;
	intptr_t pos = 0, id;
	IDAMem ida_mem = ida->mem;
	
	if (!prop) {
		return (src && ida) ? setIvp(ida, src) : MPT_ENUM(TypeSolver);
	}
	if (!(name = prop->name)) {
		if (src) {
			errno = EINVAL;
			return MPT_ERROR(BadOperation);
		}
		if ((pos = (intptr_t) prop->desc) < 0) {
			errno = EINVAL;
			return MPT_ERROR(BadArgument);
		}
	}
	else if (!*name) {
		id = MPT_SOLVER_ENUM(ODE) | MPT_SOLVER_ENUM(DAE) | MPT_SOLVER_ENUM(PDE);
		if (ida && src && (id = setIvp(ida, src)) < 0) return id;
		prop->name = "ida"; prop->desc = "DAE solver from Sundials Library";
		prop->val.fmt = "iid"; prop->val.ptr = &ida->ivp;
		return id;
	}
	if (name && !strcasecmp(name, "version")) {
		static const char version[] = MPT_VERSION"\0";
		prop->name = "version"; prop->desc = "solver release information";
		prop->val.fmt = 0; prop->val.ptr = version;
		return 0;
	}
	
	id = -1;
	if (name ? !strcasecmp(name, "atol") : (pos == id++)) {
		if (ida && (id = mpt_vecpar_value(&ida->atol, &prop->val, src)) < 0) return id;
		prop->name = "atol"; prop->desc = "absolute tolerances";
		return id;
	}
	if (name ? !strcasecmp(name, "rtol") : (pos == id++)) {
		if (ida && (id = mpt_vecpar_value(&ida->rtol, &prop->val, src)) < 0) return id;
		prop->name = "rtol"; prop->desc = "relative tolerances";
		return id;
	}
	if (name ? !strncasecmp(name, "jac", 3) : (pos == id++)) {
		if (ida && (id = sundials_jacobian(&ida->sd, ida->ivp.neqs, src)) < 0) return id;
		prop->name = "jacobian"; prop->desc = "jacobian type";
		prop->val.fmt = "B"; prop->val.ptr = &ida->sd.jacobian;
		return id;
	}
	if (name ? !strcasecmp(name, "maxord") : (pos == id++)) {
		if (ida && (id = setMaxOrd(ida, src)) < 0) return id;
		prop->name = "maxord"; prop->desc = "maximum order";
		prop->val.fmt = "i"; prop->val.ptr = ida_mem ? &ida_mem->ida_maxord : 0;
		return id;
	}
	if (name ? (!strcasecmp(name, "maxnumsteps") || !strcasecmp(name, "maxstep") || !strcasecmp(name, "mxstep")) : (pos == id++)) {
		if (ida && (id = setMaxNSteps(ida, src)) < 0) return id;
		prop->name = "maxnumsteps"; prop->desc = "maximum steps per call";
		prop->val.fmt = "l"; prop->val.ptr = ida_mem ? &ida_mem->ida_mxstep : 0;
		return id;
	}
	if (name ? (!strcasecmp(name, "stepinit") || !strcasecmp(name, "hin") || !strcasecmp(name, "h") || !strcasecmp(prop->name, "h0")) : (pos == id++)) {
		if (ida && (id = setInitStep(ida, src)) < 0) return id;
		prop->name = "hin"; prop->desc = "initial stepsize";
		prop->val.fmt = "d"; prop->val.ptr = ida_mem ? &ida_mem->ida_hin : 0;
		return id;
	}
	if (name ? (!strcasecmp(name, "hmax") || !strcasecmp(name, "stepmax")) : (pos == id++)) {
		if (ida && (id = setMaxStep(ida, src)) < 0) return id;
		prop->name = "hmax"; prop->desc = "maximal stepsize";
		prop->val.fmt= ""; prop->val.ptr = 0; /* saved as inverse */
		return id;
	}
	/* user supplied initial (dy/dt) */
	if (name ? !strncasecmp(name, "yp", 2) : (pos == id++)) {
		if (ida && (id = setYP(ida, src)) < 0) return id;
		prop->name = "yp"; prop->desc = "deviation at current time";
		prop->val.fmt= ""; prop->val.ptr = 0;
		return id;
	}
	
	errno = EINVAL;
	return MPT_ERROR(BadArgument);
}
