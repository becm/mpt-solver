/*!
 * set CVode integrator parameter
 */

#include <string.h>
#include <strings.h>

#include <cvode/cvode_impl.h>

#include "version.h"

#include "meta.h"

#include "sundials.h"

#include "module_functions.h"

static const char bdfText[] = "BDF", adamsText[] = "Adams";

static int setMethod(MPT_SOLVER_STRUCT(cvode) *cv, const MPT_INTERFACE(metatype) *src)
{
	char *val;
	int len;
	CVodeMem cv_mem = cv->mem;
	if (!src) {
		return 0;
	}
	if ((len = src->_vptr->conv(src, 'k', &val)) < 0) {
		return len;
	}
	if (!val) {
		return -2;
	}
	if (*val && !len) {
		len = strlen(val);
	}
	if (!len || !strncasecmp(adamsText, val, len)) {
		if (cv_mem->cv_MallocDone && cv_mem->cv_lmm != CV_ADAMS) {
			return MPT_ERROR(BadOperation);
		}
		cv_mem->cv_lmm  = CV_ADAMS;
		cv_mem->cv_qmax = ADAMS_Q_MAX;
	}
	else if (!strncasecmp(bdfText, val, len)) {
		if (cv_mem->cv_MallocDone && cv_mem->cv_lmm != CV_BDF) {
			return MPT_ERROR(BadOperation);
		}
		cv_mem->cv_lmm  = CV_BDF;
		cv_mem->cv_qmax = BDF_Q_MAX;
	}
	return len;
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
extern int mpt_sundials_cvode_set(MPT_SOLVER_STRUCT(cvode) *cv, const char *name, const MPT_INTERFACE(metatype) *src)
{
	CVodeMem cv_mem;
	int ret = 0;
	
	if (!cv || !(cv_mem = cv->mem)) {
		return MPT_ERROR(BadArgument);
	}
	if (!name) {
		return MPT_SOLVER_MODULE_FCN(ivp_state)(&cv->ivp, &cv->t, &cv->sd.y, src);
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
		long val = cv_mem->cv_qmax;
		if (src && (ret = src->_vptr->conv(src, 'l', &val)) < 0) return ret;
		else if (CVodeSetMaxOrd(cv_mem, val) < 0) return MPT_ERROR(BadValue);
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "mxstep") || !strcasecmp(name, "maxstep") || !strcasecmp(name, "maxnumsteps")) {
		long val = 0;
		if (src && (ret = src->_vptr->conv(src, 'l', &val)) < 0) {
			return ret;
		}
		if (val < 0) {
			cv->sd.step = CV_ONE_STEP;
		}
		else if (CVodeSetMaxNumSteps(cv_mem, val) < 0) {
			return MPT_ERROR(BadValue);
		} else {
			cv->sd.step = CV_NORMAL;
		}
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "hnilwarns")) {
		long val = 0;
		if (src && (ret = src->_vptr->conv(src, 'l', &val)) < 0) return ret;
		if (CVodeSetMaxHnilWarns(cv_mem, val) < 0) return MPT_ERROR(BadValue);
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "stepinit") || !strcasecmp(name, "h") || !strcasecmp(name, "hin") || !strcasecmp(name, "h0")) {
		double val = 0.0;
		if (src && (ret = src->_vptr->conv(src, 'd', &val)) < 0) return ret;
		if (CVodeSetInitStep(cv_mem, val) < 0) return MPT_ERROR(BadValue);
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "hmin") || !strcasecmp(name, "stepmin")) {
		double val = 0.0;
		if (src && (ret = src->_vptr->conv(src, 'd', &val)) < 0) return ret;
		if (CVodeSetMinStep(cv_mem, val) < 0) return MPT_ERROR(BadValue);
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "hmax") || !strcasecmp(name, "stepmax")) {
		double val = 0.0;
		if (src && (ret = src->_vptr->conv(src, 'd', &val)) < 0) return ret;
		if (CVodeSetMaxStep(cv_mem, val) < 0) return MPT_ERROR(BadValue);
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
	static const uint8_t longfmt[] = { 'l', 0 };
	static const uint8_t realfmt[] = { MPT_SOLVER_SUNDIALS(Realtype), 0 };
	const char *name;
	intptr_t pos = 0, id;
	CVodeMem cv_mem = cv ? cv->mem : 0;
	
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
		prop->name = "version";
		prop->desc = "solver release information";
		prop->val.fmt = 0;
		prop->val.ptr = version;
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
		static const uint8_t fmt[] = "y";
		prop->name = "jacobian";
		prop->desc = "jacobian type";
		prop->val.fmt = fmt;
		prop->val.ptr = &cv->sd.jacobian;
		return cv ? (cv->sd.jacobian ? 1 : 0) : id;
	}
	if (name ? !strcasecmp(name, "method") : (pos == ++id)) {
		prop->name = "method";
		prop->desc = "solver method";
		prop->val.fmt = 0;
		prop->val.ptr = 0;
		if (!cv_mem) return id;
		switch (cv_mem->cv_lmm) {
		  case CV_BDF: prop->val.ptr = bdfText; break;
		  case CV_ADAMS: prop->val.ptr = adamsText; break;
		  default: prop->val.ptr = "";
		}
		return id;
	}
	if (name ? !strcasecmp(name, "maxord") : (pos == ++id)) {
		const int *ptr = cv_mem ? &cv_mem->cv_qmax : 0;
		prop->name = "maxord";
		prop->desc = "maximum order of steps";
		mpt_solver_module_value_int(&prop->val, ptr);
		if (!cv_mem) return id;
		return *ptr == ADAMS_Q_MAX ? 0 : 1;
	}
	if (name ? (!strcasecmp(name, "mxstep") || !strcasecmp(name, "maxstep") || !strcasecmp(name, "maxnumsteps")) : (pos == ++id)) {
		const long *ptr = cv_mem ? &cv_mem->cv_mxstep : 0;
		prop->name = "mxstep";
		prop->desc = "allowed function evaluations per call";
		prop->val.fmt = longfmt;
		prop->val.ptr = ptr;
		if (!cv_mem) return id;
		return *ptr ? 1 : 0;
	}
	if (name ? !strcasecmp(name, "hnilwarns") : (pos == ++id)) {
		const int *ptr = cv_mem ? &cv_mem->cv_mxhnil : 0;
		prop->name = "hnilwarns";
		prop->desc = "max. warnings for 't + h == t'";
		mpt_solver_module_value_int(&prop->val, ptr);
		if (!cv_mem) return id;
		return *ptr ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "stepinit") || !strcasecmp(name, "h") || !strcasecmp(name, "hin") || !strcasecmp(name, "h0")) : (pos == ++id)) {
		const double *ptr = cv_mem ? &cv_mem->cv_hin : 0;
		prop->name = "hin";
		prop->desc = "initial stepsize";
		mpt_solver_module_value_double(&prop->val, ptr);
		if (!cv_mem) return id;
		return *ptr ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "hmin") || !strcasecmp(name, "stepmin")) : (pos == ++id)) {
		const double *ptr = cv_mem ? &cv_mem->cv_hmin : 0;
		prop->name = "hmin";
		prop->desc = "minimal stepsize";
		mpt_solver_module_value_double(&prop->val, ptr);
		if (!cv_mem) return id;
		return cv_mem->cv_hmin ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "hmax") || !strcasecmp(name, "stepmax")) : (pos == ++id)) {
		const realtype *ptr = &cv->hmax;
		prop->name = "hmax";
		prop->desc = "maximal stepsize";
		prop->val.fmt = realfmt;
		prop->val.ptr = ptr;
		if (!cv) return id;
		return *ptr ? 1 : 0;
	}
	return MPT_ERROR(BadArgument);
}
