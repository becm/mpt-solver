/*!
 * set IDA integrator parameter
 */

#include <string.h>
#include <strings.h>

#include <math.h>

#include <ida/ida_impl.h>

#include "version.h"

#include "meta.h"

#include "sundials.h"

static int setYP(MPT_SOLVER_STRUCT(ida) *ida, MPT_INTERFACE(metatype) *src)
{
	N_Vector yprime;
	double *yp;
	long pos, len;
	
	if ((len = ida->ivp.neqs * (ida->ivp.pint + 1)) < 0) {
		return MPT_ERROR(BadArgument);
	}
	if (!ida->sd.y && !(ida->sd.y = sundials_nvector_new(len))) {
		return MPT_ERROR(BadOperation);
	}
	if (!(yprime = N_VClone(ida->sd.y))) {
		return MPT_ERROR(BadOperation);
	}
	yp = N_VGetArrayPointer(yprime);
	
	pos = 0;
	if (src) {
		while (pos < len) {
			int ret;
			if ((ret = src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), yp+pos)) < 0) {
				N_VDestroy(yprime);
				return MPT_ERROR(BadType);
			}
			if (!ret) {
				break;
			}
			++pos;
		}
	}
	while (pos < len) {
		yp[pos++] = 0.0;
	}
	return pos;
}

/*!
 * \ingroup mptSundialsIda
 * \brief set IDA property
 * 
 * Query property of IDA solver
 * 
 * \param ida  IDA data
 * \param name name of property to change
 * \param src  data source to change property
 * 
 * \retval <0   failure
 * \retval >=0  used values
 */
extern int sundials_ida_set(MPT_SOLVER_STRUCT(ida) *ida, const char *name, MPT_INTERFACE(metatype) *src)
{
	IDAMem ida_mem;
	int ret = 0;
	
	if (!ida || !(ida_mem = ida->mem)) {
		return MPT_ERROR(BadArgument);
	}
	if (!name) {
		realtype t = 0;
		long required = ida->ivp.pint + 1;
		if (src && (ret = src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), &t) <= 0)) {
			if (ret < 0) {
				return ret;
			}
			src = 0;
		}
		if ((ret = sundials_vector_set(&ida->sd.y, required * ida->ivp.neqs, src)) < 0) {
			return ret;
		}
		if (src) {
			++ret;
		}
		ida->t = t;
		return ret;
	}
	if (!*name) {
		if (src && (ret = mpt_ivppar_set(&ida->ivp, src)) < 0) {
			return ret;
		}
		sundials_ida_reset(ida);
		return ret;
	}
	if (!strcasecmp(name, "atol")) {
		return mpt_vecpar_settol(&ida->atol, src, __MPT_IVP_ATOL);
	}
	if (!strcasecmp(name, "rtol")) {
		return mpt_vecpar_settol(&ida->rtol, src, __MPT_IVP_RTOL);
	}
	if (!strncasecmp(name, "jac", 3)) {
		return sundials_jacobian(&ida->sd, ida->ivp.neqs, src);
	}
	if (!strcasecmp(name, "maxord")) {
		long val = 0;
		if (src && (ret = src->_vptr->conv(src, 'l', &val)) < 0) return ret;
		if (IDASetMaxOrd(ida_mem, val) < 0) return MPT_ERROR(BadValue);
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "maxnumsteps") || !strcasecmp(name, "maxstep") || !strcasecmp(name, "mxstep")) {
		long val = 0;
		if (src && (ret = src->_vptr->conv(src, 'l', &val)) < 0) return ret;
		if (IDASetMaxNumSteps(ida_mem, val) < 0) return MPT_ERROR(BadValue);
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "tstop")) {
		double val = 0;
		if (src && (ret = src->_vptr->conv(src, 'd', &val)) < 0) return ret;
		if (ret && val) {
			if (IDASetStopTime(ida_mem, val) < 0) return MPT_ERROR(BadValue);
		} else {
			IDASetStopTime(ida_mem, INFINITY);
		}
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "stepinit") || !strcasecmp(name, "hin") || !strcasecmp(name, "h") || !strcasecmp(name, "h0")) {
		double val = 0;
		if (src && (ret = src->_vptr->conv(src, 'd', &val)) < 0) return ret;
		if (IDASetInitStep(ida_mem, val) < 0) return MPT_ERROR(BadValue);
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "hmax") || !strcasecmp(name, "stepmax")) {
		double val = 0;
		if (src && (ret = src->_vptr->conv(src, 'd', &val)) < 0) return ret;
		if (IDASetMaxStep(ida_mem, val) < 0) return MPT_ERROR(BadValue);
		ida->hmax = val;
		return ret ? 1 : 0;
	}
	/* user supplied initial (dy/dt) */
	if (!strncasecmp(name, "yp", 2)) {
		return setYP(ida, src);
	}
	return MPT_ERROR(BadArgument);
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
extern int sundials_ida_get(const MPT_SOLVER_STRUCT(ida) *ida, MPT_STRUCT(property) *prop)
{
	static const char longfmt[] = { 'l', 0 };
	static const char realfmt[] = { MPT_SOLVER_ENUM(SundialsRealtype), 0 };
	static const char dblfmt[] = { 'd', 0 };
	const char *name;
	intptr_t pos = 0, id;
	IDAMem ida_mem = ida->mem;
	
	if (!prop) {
		return MPT_ENUM(TypeSolver);
	}
	if (!(name = prop->name)) {
		if ((pos = (intptr_t) prop->desc) < 0) {
			return MPT_ERROR(BadArgument);
		}
	}
	else if (!*name) {
		prop->name = "ida"; prop->desc = "DAE solver from Sundials Library";
		prop->val.fmt = "ii"; prop->val.ptr = &ida->ivp;
		return MPT_SOLVER_ENUM(ODE) | MPT_SOLVER_ENUM(DAE) | MPT_SOLVER_ENUM(PDE);
	}
	else if (!strcasecmp(name, "version")) {
		static const char version[] = BUILD_VERSION"\0";
		prop->name = "version"; prop->desc = "solver release information";
		prop->val.fmt = 0; prop->val.ptr = version;
		return 0;
	}
	
	id = -1;
	if (name ? !strcasecmp(name, "atol") : (pos == id++)) {
		if (!ida) { prop->val.fmt = dblfmt; prop->val.ptr = &ida->atol.d.val; }
		else { id = mpt_vecpar_get(&ida->atol, &prop->val); }
		prop->name = "atol"; prop->desc = "absolute tolerances";
		return id;
	}
	if (name ? !strcasecmp(name, "rtol") : (pos == id++)) {
		if (!ida) { prop->val.fmt = dblfmt; prop->val.ptr = &ida->rtol.d.val; }
		else { id = mpt_vecpar_get(&ida->rtol, &prop->val); }
		prop->name = "rtol"; prop->desc = "relative tolerances";
		return id;
	}
	if (name ? !strncasecmp(name, "jac", 3) : (pos == id++)) {
		prop->name = "jacobian"; prop->desc = "jacobian type";
		prop->val.fmt = "B"; prop->val.ptr = &ida->sd.jacobian;
		if (!ida) return id;
		return ida->sd.jacobian ? 1 : 0;
	}
	if (name ? !strcasecmp(name, "maxord") : (pos == id++)) {
		prop->name = "maxord"; prop->desc = "maximum order";
		prop->val.fmt = "i"; prop->val.ptr = 0;
		if (!ida_mem) return id;
		prop->val.ptr = &ida_mem->ida_maxord;
		return ida_mem->ida_maxord == MAXORD_DEFAULT ? 0 : 1;
	}
	if (name ? !strcasecmp(name, "tstop") : (pos == id++)) {
		prop->name = "tstop"; prop->desc = "end time limit";
		prop->val.fmt = realfmt; prop->val.ptr = 0;
		if (!ida_mem) return id;
		prop->val.ptr = &ida_mem->ida_tstop;
		return ida_mem->ida_tstopset ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "maxnumsteps") || !strcasecmp(name, "maxstep") || !strcasecmp(name, "mxstep")) : (pos == id++)) {
		prop->name = "maxnumsteps"; prop->desc = "maximum steps per call";
		prop->val.fmt = longfmt; prop->val.ptr = 0;
		if (!ida_mem) return id;
		prop->val.ptr = &ida_mem->ida_mxstep;
		return ida_mem->ida_mxstep == MXSTEP_DEFAULT ? 0 : 1;
	}
	if (name ? (!strcasecmp(name, "stepinit") || !strcasecmp(name, "hin") || !strcasecmp(name, "h") || !strcasecmp(prop->name, "h0")) : (pos == id++)) {
		prop->name = "hin"; prop->desc = "initial stepsize";
		prop->val.fmt = "d"; prop->val.ptr = 0;
		if (!ida_mem) return id;
		prop->val.ptr = &ida_mem->ida_hin;
		return ida_mem->ida_hin ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "hmax") || !strcasecmp(name, "stepmax")) : (pos == id++)) {
		prop->name = "hmax"; prop->desc = "maximal stepsize";
		prop->val.fmt= realfmt; prop->val.ptr = &ida->hmax;
		if (!ida) return id;
		return ida->hmax ? 1 : 0;
	}
	/* user supplied initial (dy/dt) */
	if (name ? !strncasecmp(name, "yp", 2) : (pos == id++)) {
		prop->name = "yp"; prop->desc = "deviation at current time";
		prop->val.fmt= ""; prop->val.ptr = 0;
		return ida->ivp.neqs * (ida->ivp.pint + 1);
	}
	return MPT_ERROR(BadArgument);
}
