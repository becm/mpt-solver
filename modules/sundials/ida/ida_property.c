/*!
 * set IDA integrator parameter
 */

#include <string.h>
#include <strings.h>

#include <math.h>

#include <ida/ida.h>

#include "version.h"

#include "types.h"
#include "meta.h"

#include "sundials.h"

#include "module_functions.h"

static int setYP(MPT_SOLVER_STRUCT(ida) *ida, MPT_INTERFACE(convertable) *src)
{
	MPT_INTERFACE(iterator) *it;
	N_Vector yprime;
	realtype *yp;
	long pos, len;
	
	it = 0;
	if (src && (pos = src->_vptr->convert(src, MPT_ENUM(TypeIteratorPtr), &it)) < 0) {
		return pos;
	}
	if ((len = ida->ivp.neqs * (ida->ivp.pint + 1)) < 0) {
		return MPT_ERROR(BadArgument);
	}
	if (!ida->sd.y) {
#if SUNDIALS_VERSION_MAJOR >= 6
		if (!(ida->sd.y = mpt_sundials_nvector(len, mpt_sundials_context(&ida->sd)))) {
#else
		if (!(ida->sd.y = mpt_sundials_nvector(len))) {
#endif
			return MPT_ERROR(BadOperation);
		}
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

static void *getSolverData(MPT_SOLVER_STRUCT(ida) *ida) {
	void *ida_mem;
	
	if ((ida_mem = ida->mem)) {
		return ida_mem;
	}
#if SUNDIALS_VERSION_MAJOR >= 6
	if (!(ida_mem = IDACreate(mpt_sundials_context(&ida->sd)))) {
#else
	if (!(ida_mem = IDACreate())) {
#endif
		return NULL;
	}
	if (IDASetUserData(ida_mem, ida) != IDA_SUCCESS) {
		IDAFree(&ida_mem);
		return NULL;
	}
	ida->mem = ida_mem;
	
	return ida_mem;
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
extern int mpt_sundials_ida_set(MPT_SOLVER_STRUCT(ida) *ida, const char *name, MPT_INTERFACE(convertable) *src)
{
	int ret = 0;
	
	if (!ida) {
		return MPT_ERROR(BadArgument);
	}
	if (!name) {
		MPT_SOLVER_STRUCT(sundials_vector_context) ctx = {
			&ida->sd.y
#if SUNDIALS_VERSION_MAJOR >= 6
			, mpt_sundials_context(&ida->sd)
#endif
		};
		return MPT_SOLVER_MODULE_FCN(ivp_state)(&ida->ivp, &ida->t, &ctx, src);
	}
	if (!*name) {
		if (src && (ret = mpt_solver_module_ivpset(&ida->ivp, src)) < 0) {
			return ret;
		}
		mpt_sundials_ida_reset(ida);
		return ret;
	}
	if (!strcasecmp(name, "atol")) {
		return mpt_solver_module_tol_set(&ida->atol, src, __MPT_IVP_ATOL);
	}
	if (!strcasecmp(name, "rtol")) {
		return mpt_solver_module_tol_set(&ida->rtol, src, __MPT_IVP_RTOL);
	}
	if (!strncasecmp(name, "jac", 3)) {
		return mpt_sundials_jacobian(&ida->sd, src);
	}
	if (!strcasecmp(name, "maxord")) {
		long val = 0;
		if (src && (ret = src->_vptr->convert(src, 'l', &val)) < 0) {
			return ret;
		}
		if (IDASetMaxOrd(getSolverData(ida), val) < 0) {
			return MPT_ERROR(BadValue);
		}
		ida->maxord = val;
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "mxstep") || !strcasecmp(name, "maxnumsteps") || !strcasecmp(name, "maxstep")) {
		long val = 0;
		if (src && (ret = src->_vptr->convert(src, 'l', &val)) < 0) {
			return ret;
		}
		if ((val != 0) && (IDASetMaxNumSteps(getSolverData(ida), val) != IDA_SUCCESS)) {
			return MPT_ERROR(BadValue);
		}
		ida->mxstep = val;
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "tstop")) {
		double val = INFINITY;
		if (src && (ret = src->_vptr->convert(src, 'd', &val)) < 0) {
			return ret;
		}
		if (IDASetStopTime(getSolverData(ida), val) != IDA_SUCCESS) {
			return MPT_ERROR(BadValue);
		}
		ida->step.tstop = val;
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "hin") || !strcasecmp(name, "stepinit") || !strcasecmp(name, "h") || !strcasecmp(name, "h0")) {
		double val = 0;
		if (src && (ret = src->_vptr->convert(src, 'd', &val)) < 0) {
			return ret;
		}
		if (IDASetInitStep(getSolverData(ida), val) != IDA_SUCCESS) {
			return MPT_ERROR(BadValue);
		}
		ida->step.hin = val;
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "hmax") || !strcasecmp(name, "stepmax")) {
		double val = 0;
		if (src && (ret = src->_vptr->convert(src, 'd', &val)) < 0) {
			return ret;
		}
		if (IDASetMaxStep(getSolverData(ida), val) != IDA_SUCCESS) {
			return MPT_ERROR(BadValue);
		}
		ida->step.hmax = val;
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
extern int mpt_sundials_ida_get(const MPT_SOLVER_STRUCT(ida) *ida, MPT_STRUCT(property) *prop)
{
	const char *name;
	intptr_t pos = 0, id;
	
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
		return mpt_solver_module_value_ivp(prop, ida ? &ida->ivp : 0);
	}
	else if (!strcasecmp(name, "version")) {
		static const char version[] = BUILD_VERSION"\0";
		const char *ptr = version;
		prop->name = "version";
		prop->desc = "solver release information";
		mpt_solver_module_value_string(prop, ptr);
		return 0;
	}
	
	id = -1;
	if (name ? !strcasecmp(name, "atol") : (pos == id++)) {
		prop->name = "atol";
		prop->desc = "absolute tolerances";
		if (!ida) {
			MPT_value_set(&prop->val, 'd', 0);
			return id;
		}
		return mpt_solver_module_tol_get(prop, &ida->atol);
	}
	if (name ? !strcasecmp(name, "rtol") : (pos == id++)) {
		prop->name = "rtol";
		prop->desc = "relative tolerances";
		if (!ida) {
			MPT_value_set(&prop->val, 'd', 0);
			return id;
		}
		return mpt_solver_module_tol_get(prop, &ida->rtol);
	}
	if (name ? !strncasecmp(name, "jac", 3) : (pos == id++)) {
		const int8_t *ptr = &ida->sd.jacobian;
		const int type = 'b';
		prop->name = "jacobian";
		prop->desc = "jacobian type";
		if (!ida) {
			MPT_value_set(&prop->val, type, 0);
			return id;
		}
		mpt_solver_module_value_set(prop, type, ptr, sizeof(*ptr));
		return *ptr ? 1 : 0;
	}
	if (name ? !strcasecmp(name, "maxord") : (pos == id++)) {
		const int *ptr = &ida->maxord;
		prop->name = "maxord";
		prop->desc = "maximum order";
		if (!ida) {
			MPT_value_set(&prop->val, 'i', 0);
			return id;
		}
		mpt_solver_module_value_int(prop, ptr);
		return (ida->maxord >= 0) ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "mxstep") || !strcasecmp(name, "maxnumsteps") || !strcasecmp(name, "maxstep")) : (pos == id++)) {
		const long *ptr = &ida->mxstep;
		prop->name = "mxstep";
		prop->desc = "maximum steps per call";
		if (!ida) {
			MPT_value_set(&prop->val, 'l', 0);
			return id;
		}
		mpt_solver_module_value_long(prop, ptr);
		return (ida->mxstep >= 0) ? 1 : 0;
	}
	if (name ? !strcasecmp(name, "tstop") : (pos == id++)) {
		const realtype *ptr = &ida->step.tstop;
		const int type = MPT_SOLVER_SUNDIALS(Realtype);
		prop->name = "tstop";
		prop->desc = "end time limit";
		if (!ida) {
			MPT_value_set(&prop->val, type, 0);
			return id;
		}
		mpt_solver_module_value_set(prop, type, ptr, sizeof(*ptr));
		return (ida->step.tstop != INFINITY) ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "hin") || !strcasecmp(name, "stepinit") || !strcasecmp(name, "h") || !strcasecmp(prop->name, "h0")) : (pos == id++)) {
		const realtype *ptr = &ida->step.hin;
		const int type = MPT_SOLVER_SUNDIALS(Realtype);
		prop->name = "hin";
		prop->desc = "initial stepsize";
		if (!ida) {
			MPT_value_set(&prop->val, type, 0);
			return id;
		}
		mpt_solver_module_value_set(prop, type, ptr, sizeof(*ptr));
		return (ida->step.hin != 0.0) ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "hmax") || !strcasecmp(name, "stepmax")) : (pos == id++)) {
		const realtype *ptr = &ida->step.hmax;
		const int type = MPT_SOLVER_SUNDIALS(Realtype);
		prop->name = "hmax";
		prop->desc = "maximal stepsize";
		if (!ida) {
			MPT_value_set(&prop->val, type, 0);
			return id;
		}
		mpt_solver_module_value_set(prop, type, ptr, sizeof(*ptr));
		return (ida->step.hmax != 0.0) ? 1 : 0;
	}
	/* user supplied initial (dy/dt) */
	if (name ? !strncasecmp(name, "yp", 2) : (pos == id++)) {
		const int type = MPT_type_toVector((int) MPT_SOLVER_SUNDIALS(Realtype));
		const realtype *ptr;
		
		prop->name = "yp";
		prop->desc = "deviation at current time";
		if (!ida || !ida->yp || !(ptr = N_VGetArrayPointer(ida->yp))) {
			MPT_value_set(&prop->val, type , 0);
			return id;
		}
		else {
			struct iovec vec;
			vec.iov_base = (void *) ptr;
			vec.iov_len  = (ida->ivp.neqs * (ida->ivp.pint + 1)) * sizeof(*ptr);
			if ((id = mpt_solver_module_value_set(prop, type, &vec, sizeof(vec))) < 0) {
				return id;
			}
			return ida->ivp.neqs * (ida->ivp.pint + 1);
		}
	}
	return MPT_ERROR(BadArgument);
}
