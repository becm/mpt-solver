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

#include "module_functions.h"

static int setYP(MPT_SOLVER_STRUCT(ida) *ida, const MPT_INTERFACE(metatype) *src)
{
	MPT_INTERFACE(iterator) *it;
	N_Vector yprime;
	realtype *yp;
	long pos, len;
	
	it = 0;
	if (src && (pos = src->_vptr->conv(src, MPT_ENUM(TypeIterator), &it)) < 0) {
		return pos;
	}
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
	
	if ((len = ida->ivp.pint)) {
		++len;
	}
	return MPT_SOLVER_MODULE_FCN(data_set)(yp, ida->ivp.neqs, len, it);
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
extern int sundials_ida_set(MPT_SOLVER_STRUCT(ida) *ida, const char *name, const MPT_INTERFACE(metatype) *src)
{
	IDAMem ida_mem;
	int ret = 0;
	
	if (!ida || !(ida_mem = ida->mem)) {
		return MPT_ERROR(BadArgument);
	}
	if (!name) {
		return MPT_SOLVER_MODULE_FCN(ivp_state)(&ida->ivp, &ida->t, &ida->sd.y, src);
	}
	if (!*name) {
		if (src && (ret = mpt_solver_module_ivpset(&ida->ivp, src)) < 0) {
			return ret;
		}
		sundials_ida_reset(ida);
		return ret;
	}
	if (!strcasecmp(name, "atol")) {
		return mpt_solver_module_tol_set(&ida->atol, src, __MPT_IVP_ATOL);
	}
	if (!strcasecmp(name, "rtol")) {
		return mpt_solver_module_tol_set(&ida->rtol, src, __MPT_IVP_RTOL);
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
	static const uint8_t longfmt[] = { 'l', 0 };
	static const uint8_t realfmt[] = { MPT_SOLVER_ENUM(SundialsRealtype), 0 };
	const char *name;
	intptr_t pos = 0, id;
	IDAMem ida_mem = ida->mem;
	
	if (!prop) {
		return MPT_SOLVER_ENUM(DAE) | MPT_SOLVER_ENUM(PDE);
	}
	if (!(name = prop->name)) {
		if ((pos = (intptr_t) prop->desc) < 0) {
			return MPT_ERROR(BadArgument);
		}
	}
	else if (!*name) {
		prop->name = "ida";
		prop->desc = "DAE solver from Sundials Library";
		return mpt_solver_module_value_ivp(&prop->val, ida ? &ida->ivp : 0);
	}
	else if (!strcasecmp(name, "version")) {
		static const char version[] = BUILD_VERSION"\0";
		prop->name = "version";
		prop->desc = "solver release information";
		prop->val.fmt = 0;
		prop->val.ptr = version;
		return 0;
	}
	
	id = -1;
	if (name ? !strcasecmp(name, "atol") : (pos == id++)) {
		prop->name = "atol";
		prop->desc = "absolute tolerances";
		if (ida) {
			return mpt_solver_module_tol_get(&prop->val, &ida->atol);
		}
		mpt_solver_module_value_double(&prop->val, &ida->atol.d.val);
		return id;
	}
	if (name ? !strcasecmp(name, "rtol") : (pos == id++)) {
		prop->name = "rtol";
		prop->desc = "relative tolerances";
		if (ida) {
			return mpt_solver_module_tol_get(&prop->val, &ida->rtol);
		}
		mpt_solver_module_value_double(&prop->val, &ida->rtol.d.val);
		return id;
	}
	if (name ? !strncasecmp(name, "jac", 3) : (pos == id++)) {
		static const uint8_t fmt[] = "b";
		const int8_t *ptr = &ida->sd.jacobian;
		prop->name = "jacobian";
		prop->desc = "jacobian type";
		prop->val.fmt = fmt;
		prop->val.ptr = ptr;
		if (!ida) return id;
		return *ptr ? 1 : 0;
	}
	if (name ? !strcasecmp(name, "maxord") : (pos == id++)) {
		const int *ptr = ida_mem ? &ida_mem->ida_maxord : 0;
		prop->name = "maxord";
		prop->desc = "maximum order";
		mpt_solver_module_value_int(&prop->val, ptr);
		if (!ida_mem) return id;
		return *ptr == MAXORD_DEFAULT ? 0 : 1;
	}
	if (name ? !strcasecmp(name, "tstop") : (pos == id++)) {
		const realtype *ptr = ida_mem ? &ida_mem->ida_tstop : 0;
		prop->name = "tstop";
		prop->desc = "end time limit";
		prop->val.fmt = realfmt;
		prop->val.ptr = ptr;
		if (!ida_mem) return id;
		return *ptr ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "maxnumsteps") || !strcasecmp(name, "maxstep") || !strcasecmp(name, "mxstep")) : (pos == id++)) {
		const long *ptr = ida_mem ? &ida_mem->ida_mxstep : 0;
		prop->name = "maxnumsteps";
		prop->desc = "maximum steps per call";
		prop->val.fmt = longfmt;
		prop->val.ptr = ptr;
		if (!ida_mem) return id;
		return *ptr == MXSTEP_DEFAULT ? 0 : 1;
	}
	if (name ? (!strcasecmp(name, "stepinit") || !strcasecmp(name, "hin") || !strcasecmp(name, "h") || !strcasecmp(prop->name, "h0")) : (pos == id++)) {
		const double *ptr = ida_mem ? &ida_mem->ida_hin : 0;
		prop->name = "hin";
		prop->desc = "initial stepsize";
		mpt_solver_module_value_double(&prop->val, ptr);
		if (!ida_mem) return id;
		return *ptr ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "hmax") || !strcasecmp(name, "stepmax")) : (pos == id++)) {
		prop->name = "hmax";
		prop->desc = "maximal stepsize";
		prop->val.fmt= realfmt;
		prop->val.ptr = &ida->hmax;
		if (!ida) return id;
		return ida->hmax ? 1 : 0;
	}
	/* user supplied initial (dy/dt) */
	if (name ? !strncasecmp(name, "yp", 2) : (pos == id++)) {
		prop->name = "yp";
		prop->desc = "deviation at current time";
		prop->val.fmt = 0;
		prop->val.ptr = 0;
		return ida->ivp.neqs * (ida->ivp.pint + 1);
	}
	return MPT_ERROR(BadArgument);
}
