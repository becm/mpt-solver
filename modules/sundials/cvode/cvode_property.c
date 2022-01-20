/*!
 * set CVode integrator parameter
 */

#include <string.h>
#include <strings.h>

#include <math.h>

#include <cvode/cvode.h>

#include "version.h"

#include "meta.h"

#include "sundials.h"

#include "module_functions.h"

static const char bdfText[] = "BDF", adamsText[] = "Adams";

static int setMethod(MPT_SOLVER_STRUCT(cvode) *cv, MPT_INTERFACE(convertable) *src)
{
	char *val;
	int len;
	
	if (cv->mem) {
		return MPT_ERROR(BadOperation);
	}
	if (!src) {
		return 0;
	}
	if ((len = src->_vptr->convert(src, 'k', &val)) < 0) {
		return len;
	}
	if (!val) {
		return -2;
	}
	if (*val && !len) {
		len = strlen(val);
	}
	if (!len || !strncasecmp(adamsText, val, sizeof(adamsText))) {
		cv->method = CV_ADAMS;
	}
	else if (!strncasecmp(bdfText, val, len)) {
		cv->method = CV_BDF;
	}
	return len;
}

static void *getSolverData(MPT_SOLVER_STRUCT(cvode) *cv) {
	void *cv_mem;
	
	if ((cv_mem = cv->mem)) {
		return cv_mem;
	}
#if SUNDIALS_VERSION_MAJOR >= 6
	if (!(cv_mem = CVodeCreate(cv->method, mpt_sundials_context(&cv->sd)))) {
#else
	if (!(cv_mem = CVodeCreate(cv->method))) {
#endif
		return NULL;
	}
	if (CVodeSetUserData(cv_mem, cv) != CV_SUCCESS) {
		CVodeFree(&cv_mem);
		return NULL;
	}
	cv->mem = cv_mem;
	
	return cv_mem;
}

/*!
 * \ingroup mptSundialsCVode
 * \brief set CVode property
 * 
 * Assign property of CVode solver
 * 
 * \param cv   CVode data
 * \param name name of property to change
 * \param src  data source to change property
 * 
 * \retval <0   failure
 * \retval >=0  used values
 */
extern int mpt_sundials_cvode_set(MPT_SOLVER_STRUCT(cvode) *cv, const char *name, MPT_INTERFACE(convertable) *src)
{
	int ret = 0;
	
	if (!cv) {
		return MPT_ERROR(BadArgument);
	}
	if (!name) {
		MPT_SOLVER_STRUCT(sundials_vector_context) ctx = {
			&cv->sd.y
#if SUNDIALS_VERSION_MAJOR >= 6
			, mpt_sundials_context(&cv->sd)
#endif
		};
		return MPT_SOLVER_MODULE_FCN(ivp_state)(&cv->ivp, &cv->t, &ctx, src);
	}
	if (!*name) {
		if (src && (ret = mpt_solver_module_ivpset(&cv->ivp, src)) < 0) {
			return ret;
		}
		mpt_sundials_cvode_reset(cv);
		return ret;
	}
	if (!strcasecmp(name, "atol")) {
		return mpt_solver_module_tol_set(&cv->atol, src, __MPT_IVP_ATOL);
		return ret;
	}
	if (!strcasecmp(name, "rtol")) {
		return mpt_solver_module_tol_set(&cv->rtol, src, __MPT_IVP_RTOL);
	}
	if (!strncasecmp(name, "jac", 3)) {
		return mpt_sundials_jacobian(&cv->sd, src);
	}
	if (!strcasecmp(name, "method")) {
		return setMethod(cv, src);
	}
	if (!strcasecmp(name, "maxord")) {
		long val = 0;
		if (src && (ret = src->_vptr->convert(src, 'l', &val)) < 0) {
			return ret;
		}
		else if (CVodeSetMaxOrd(getSolverData(cv), val) != CV_SUCCESS) {
			return MPT_ERROR(BadValue);
		}
		cv->maxord = val;
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "mxstep") || !strcasecmp(name, "maxstep") || !strcasecmp(name, "maxnumsteps")) {
		long val = 0;
		if (src && (ret = src->_vptr->convert(src, 'l', &val)) < 0) {
			return ret;
		}
		if ((val != 0) && (CVodeSetMaxNumSteps(getSolverData(cv), val) != CV_SUCCESS)) {
			return MPT_ERROR(BadValue);
		}
		cv->mxstep = val;
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "hnilwarns")) {
		long val = 0;
		if (src && (ret = src->_vptr->convert(src, 'l', &val)) < 0) {
			return ret;
		}
		if (CVodeSetMaxHnilWarns(getSolverData(cv), val) != CV_SUCCESS) {
			return MPT_ERROR(BadValue);
		}
		cv->mxhnil = val;
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "tstop")) {
		double val = INFINITY;
		if (src && (ret = src->_vptr->convert(src, 'd', &val)) < 0) {
			return ret;
		}
		if (CVodeSetStopTime(getSolverData(cv), val) != CV_SUCCESS) {
			return MPT_ERROR(BadValue);
		}
		cv->step.tstop = val;
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "hin") || !strcasecmp(name, "stepinit") || !strcasecmp(name, "h") || !strcasecmp(name, "h0")) {
		double val = 0.0;
		if (src && (ret = src->_vptr->convert(src, 'd', &val)) < 0) {
			return ret;
		}
		if (CVodeSetInitStep(getSolverData(cv), val) != CV_SUCCESS) {
			return MPT_ERROR(BadValue);
		}
		cv->step.hin = val;
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "hmin") || !strcasecmp(name, "stepmin")) {
		double val = 0.0;
		if (src && (ret = src->_vptr->convert(src, 'd', &val)) < 0) {
			return ret;
		}
		if (CVodeSetMinStep(getSolverData(cv), val) != CV_SUCCESS) {
			return MPT_ERROR(BadValue);
		}
		cv->step.hmin = val;
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "hmax") || !strcasecmp(name, "stepmax")) {
		double val = 0.0;
		if (src && (ret = src->_vptr->convert(src, 'd', &val)) < 0) {
			return ret;
		}
		if (CVodeSetMaxStep(getSolverData(cv), val) != CV_SUCCESS) {
			return MPT_ERROR(BadValue);
		}
		cv->step.hmax = val;
		return ret ? 1 : 0;
	}
	return MPT_ERROR(BadArgument);
}
/*!
 * \ingroup mptSundialsCVode
 * \brief set CVode property
 * 
 * Query property of CVode solver
 * 
 * \param cv   CVode data
 * \param prop object property
 * 
 * \retval 0   default value
 * \retval <0  failure
 * \retval >0  changed property
 */
extern int mpt_sundials_cvode_get(const MPT_SOLVER_STRUCT(cvode) *cv, MPT_STRUCT(property) *prop)
{
	const char *name;
	intptr_t pos = 0, id;
	
	if (!prop) {
		return MPT_SOLVER_ENUM(ODE) | MPT_SOLVER_ENUM(PDE);
	}
	if (!(name = prop->name)) {
		if ((pos = (intptr_t) prop->desc) < 0) {
			return MPT_ERROR(BadArgument);
		}
	}
	else if (!*name) {
		prop->name = "cvode";
		prop->desc = "ODE solver from Sundials Library";
		return mpt_solver_module_value_ivp(&prop->val, cv ? &cv->ivp : 0);
	}
	else if (!strcasecmp(name, "version")) {
		static const char version[] = BUILD_VERSION"\0";
		const char *ptr = version;
		prop->name = "version";
		prop->desc = "solver release information";
		mpt_solver_module_value_string(&prop->val, ptr);
		return 0;
	}
	
	id = -1;
	if (name ? !strcasecmp(name, "atol") : (pos == ++id)) {
		prop->name = "atol";
		prop->desc = "absolute tolerances";
		if (cv) {
			return mpt_solver_module_tol_get(&prop->val, &cv->atol);
		}
		mpt_solver_module_value_double(&prop->val, &cv->atol._d.val);
		return id;
	}
	if (name ? !strcasecmp(name, "rtol") : (pos == ++id)) {
		prop->name = "rtol";
		prop->desc = "relative tolerances";
		if (cv) {
			return mpt_solver_module_tol_get(&prop->val, &cv->rtol);
		}
		mpt_solver_module_value_double(&prop->val, &cv->rtol._d.val);
		return id;
	}
	if (name ? !strncasecmp(name, "jac", 3) : (pos == ++id)) {
		prop->name = "jacobian";
		prop->desc = "jacobian type";
		prop->val.type = 'y';
		prop->val.ptr = &cv->sd.jacobian;
		if (!cv) return id;
		MPT_value_set_data(&prop->val, 'y', &cv->sd.jacobian);
		return cv ? (cv->sd.jacobian ? 1 : 0) : id;
	}
	if (name ? !strcasecmp(name, "method") : (pos == ++id)) {
		const char *val = 0;
		prop->name = "method";
		prop->desc = "solver method";
		prop->val.type = 's';
		prop->val.ptr = 0;
		if (!cv) return id;
		switch (cv->method) {
		  case CV_BDF: val = bdfText; break;
		  case CV_ADAMS: val = adamsText; break;
		  default: val = "";
		}
		mpt_solver_module_value_string(&prop->val, val);
		return id;
	}
	if (name ? !strcasecmp(name, "maxord") : (pos == ++id)) {
		prop->name = "maxord";
		prop->desc = "maximum order of steps";
		mpt_solver_module_value_int(&prop->val, &cv->maxord);
		if (!cv) return id;
		return (cv->maxord >= 0) ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "mxstep") || !strcasecmp(name, "maxstep") || !strcasecmp(name, "maxnumsteps")) : (pos == ++id)) {
		prop->name = "mxstep";
		prop->desc = "allowed function evaluations per call";
		prop->val.type = 'l';
		prop->val.ptr = &cv->mxstep;
		if (!cv) return id;
		mpt_solver_module_value_long(&prop->val, &cv->mxstep);
		return (cv->mxstep >= 0) ? 1 : 0;
	}
	if (name ? !strcasecmp(name, "hnilwarns") : (pos == ++id)) {
		prop->name = "hnilwarns";
		prop->desc = "threshold for 't + h == t' warnings";
		prop->val.type = 'i';
		prop->val.ptr = &cv->mxhnil;
		if (!cv) return id;
		mpt_solver_module_value_int(&prop->val, &cv->mxhnil);
		return (cv->mxhnil >= 0) ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "tstop") || !strcasecmp(name, "tend")) : (pos == ++id)) {
		const realtype *ptr = &cv->step.tstop;
		prop->name = "tstop";
		prop->desc = "final independent variable value";
		prop->val.type = MPT_SOLVER_SUNDIALS(Realtype);
		prop->val.ptr = ptr;
		if (!cv) return id;
		if (prop->val._bufsize >= sizeof(*ptr)) {
			prop->val.ptr = memcpy(prop->val._buf, ptr, sizeof(*ptr));
		}
		return (cv->step.tstop != INFINITY) ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "stepinit") || !strcasecmp(name, "h") || !strcasecmp(name, "hin") || !strcasecmp(name, "h0")) : (pos == ++id)) {
		const realtype *ptr = &cv->step.hin;
		prop->name = "hin";
		prop->desc = "initial stepsize";
		prop->val.type = MPT_SOLVER_SUNDIALS(Realtype);
		prop->val.ptr = ptr;
		if (!cv) return id;
		if (prop->val._bufsize >= sizeof(*ptr)) {
			prop->val.ptr = memcpy(prop->val._buf, ptr, sizeof(*ptr));
		}
		return (cv->step.hin != 0.0) ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "hmin") || !strcasecmp(name, "stepmin")) : (pos == ++id)) {
		const realtype *ptr = &cv->step.hmin;
		prop->name = "hmin";
		prop->desc = "minimal stepsize";
		prop->val.type = MPT_SOLVER_SUNDIALS(Realtype);
		prop->val.ptr = ptr;
		if (!cv) return id;
		if (prop->val._bufsize >= sizeof(*ptr)) {
			prop->val.ptr = memcpy(prop->val._buf, ptr, sizeof(*ptr));
		}
		return (cv->step.hmin != 0.0) ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "hmax") || !strcasecmp(name, "stepmax")) : (pos == ++id)) {
		const realtype *ptr = &cv->step.hmax;
		prop->name = "hmax";
		prop->desc = "maximal stepsize";
		prop->val.type = MPT_SOLVER_SUNDIALS(Realtype);
		prop->val.ptr = ptr;
		if (!cv) return id;
		if (prop->val._bufsize >= sizeof(*ptr)) {
			prop->val.ptr = memcpy(prop->val._buf, ptr, sizeof(*ptr));
		}
		return (cv->step.hmax != 0.0) ? 1 : 0;
	}
	return MPT_ERROR(BadArgument);
}
