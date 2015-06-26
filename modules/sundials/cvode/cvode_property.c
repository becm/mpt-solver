/*!
 * set CVode integrator parameter
 */

#include <string.h>
#include <errno.h>

#include <cvode/cvode_impl.h>

#include "release.h"
#include "version.h"

#include "sundials.h"

static char bdfText[] = "BDF", adamsText[] = "Adams";

static int setIvp(MPT_SOLVER_STRUCT(cvode) *data, MPT_INTERFACE(source) *src)
{
	int ret;
	if ((ret = mpt_ivppar_set(&data->ivp, src)) >= 0 && src) {
		sundials_cvode_reset(data);
	}
	return ret;
}
static int setMethod(MPT_SOLVER_STRUCT(cvode) *cv, MPT_INTERFACE(source) *src)
{
	char *val;
	int len;
	CVodeMem cv_mem = cv->mem;
	if (!src) return 0;
	if ((len = src->_vptr->conv(src, 'k', &val)) < 0) return len;
	if (!val) return -2;
	if (*val && !len) len = strlen(val);
	if (!len || !strncasecmp(adamsText, val, len)) {
		if (cv_mem->cv_MallocDone && cv_mem->cv_lmm != CV_ADAMS) return MPT_ERROR(BadOperation);
		cv_mem->cv_lmm  = CV_ADAMS; cv_mem->cv_qmax = ADAMS_Q_MAX;
	}
	else if (!strncasecmp(bdfText, val, len)) {
		if (cv_mem->cv_MallocDone && cv_mem->cv_lmm != CV_BDF) return MPT_ERROR(BadOperation);
		cv_mem->cv_lmm  = CV_BDF;   cv_mem->cv_qmax = BDF_Q_MAX;
	}
	return len;
}
static int setMaxOrd(MPT_SOLVER_STRUCT(cvode) *cv, MPT_INTERFACE(source) *src)
{
	int64_t val;
	int len;
	CVodeMem cv_mem = cv->mem;
	if (!src) return 0;
	if ((len = src->_vptr->conv(src, 'l', &val)) < 0) return len;
	else if (CVodeSetMaxOrd(cv->mem, len ? val : cv_mem->cv_qmax) < 0) return MPT_ERROR(BadValue);
	return len;
}
static int setMaxNSteps(MPT_SOLVER_STRUCT(cvode) *cv, MPT_INTERFACE(source) *src)
{
	int64_t val;
	int len;
	if (!src) return 0;
	if ((len = src->_vptr->conv(src, 'l', &val)) < 0) return len;
	if (CVodeSetMaxNumSteps(cv->mem, len ? val : 0) < 0) return MPT_ERROR(BadValue);
	return len;
}
static int setMaxHNil(MPT_SOLVER_STRUCT(cvode) *cv, MPT_INTERFACE(source) *src)
{
	int64_t val;
	int len;
	if (!src) return 0;
	if ((len = src->_vptr->conv(src, 'l', &val)) < 0) return len;
	if (CVodeSetMaxHnilWarns(cv->mem, len ? val : 0) < 0) return MPT_ERROR(BadValue);
	return len;
}
static int setInitStep(MPT_SOLVER_STRUCT(cvode) *cv, MPT_INTERFACE(source) *src)
{
	double val;
	int len;
	if (!src) return 0;
	if ((len = src->_vptr->conv(src, 'd', &val)) < 0) return len;
	if (CVodeSetInitStep(cv->mem, len ? val : 0) < 0) return MPT_ERROR(BadValue);
	return len;
}
static int setMinStep(MPT_SOLVER_STRUCT(cvode) *cv, MPT_INTERFACE(source) *src)
{
	double val;
	int len;
	if (!src) return 0;
	if ((len = src->_vptr->conv(src, 'd', &val)) < 0) return len;
	if (CVodeSetMinStep(cv->mem, len ? val : 0) < 0) return MPT_ERROR(BadValue);
	return len;
}
static int setMaxStep(MPT_SOLVER_STRUCT(cvode) *cv, MPT_INTERFACE(source) *src)
{
	double val;
	int len;
	if (!src) return 0;
	if ((len = src->_vptr->conv(src, 'd', &val)) < 0) return len;
	if (CVodeSetMaxStep(cv->mem, len ? val : 0) < 0) return MPT_ERROR(BadValue);
	return len;
}

/*!
 * \ingroup mptSundialsCVode
 * \brief set CVode property
 * 
 * Query property of CVode solver
 * 
 * \param cv   CVode data
 * \param prop metatype property
 * \param src  data source to change property
 * 
 * \retval 0   default value
 * \retval <0  failure
 * \retval >0  changed property
 */
extern int sundials_cvode_property(MPT_SOLVER_STRUCT(cvode) *cv, MPT_STRUCT(property) *prop, MPT_INTERFACE(source) *src)
{
	const char *name;
	intptr_t pos = 0, id;
	CVodeMem cv_mem = cv ? cv->mem : 0;
	
	if (!prop) {
		return (src && cv) ? setIvp(cv, src) : MPT_ENUM(TypeSolver);
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
		id = MPT_SOLVER_ENUM(ODE) | MPT_SOLVER_ENUM(PDE);
		if (cv && src && (id = setIvp(cv, src)) < 0) return id;
		prop->name = "cvode"; prop->desc = "ODE solver from Sundials Library";
		prop->val.fmt = "iid"; prop->val.ptr = &cv->ivp;
		return id;
	}
	if (name && !strcasecmp(name, "version")) {
		static const char version[] = MPT_VERSION"\0";
		prop->name = "version"; prop->desc = "solver release information";
		prop->val.fmt = 0; prop->val.ptr = version;
		return 0;
	}
	
	id = -1;
	if (name ? !strcasecmp(name, "atol") : (pos == ++id)) {
		if (cv && (id = mpt_vecpar_value(&cv->atol, &prop->val, src)) < 0) return id;
		prop->name = "atol"; prop->desc = "absolute tolerances";
		return id;
	}
	if (name ? !strcasecmp(name, "rtol") : (pos == ++id)) {
		if (cv && (id = mpt_vecpar_value(&cv->rtol, &prop->val, src)) < 0) return id;
		prop->name = "rtol"; prop->desc = "relative tolerances";
		return id;
	}
	if (name ? !strncasecmp(name, "jac", 3) : (pos == ++id)) {
		if (cv && (id = sundials_jacobian(&cv->sd, cv->ivp.neqs, src)) < 0) return id;
		prop->name = "jacobian"; prop->desc = "jacobian type";
		prop->val.fmt = "B"; prop->val.ptr = &cv->sd.jacobian;
		return id;
	}
	if (name ? !strcasecmp(name, "method") : (pos == ++id)) {
		if (cv && (id = setMethod(cv, src)) < 0) return id;
		prop->name = "method"; prop->desc = "solver method";
		prop->val.fmt = prop->val.ptr = 0;
		if (!cv) return id;
		switch (cv_mem->cv_lmm) {
		  case CV_BDF: prop->val.ptr = bdfText; break;
		  case CV_ADAMS: prop->val.ptr = adamsText; break;
		  default: prop->val.ptr = "";
		}
		return id;
	}
	if (name ? !strcasecmp(name, "maxord") : (pos == ++id)) {
		if (cv && (id = setMaxOrd(cv, src)) < 0) return id;
		prop->name = "maxord"; prop->desc = "maximum order of steps";
		prop->val.fmt = "i"; prop->val.ptr = 0;
		return id;
	}
	if (name ? (!strcasecmp(name, "mxstep") || !strcasecmp(name, "maxstep") || !strcasecmp(name, "maxnumsteps")) : (pos == ++id)) {
		if (cv && (id = setMaxNSteps(cv, src)) < 0) return id;
		prop->name = "mxstep"; prop->desc = "allowed function evaluations per call";
		prop->val.fmt = "l"; prop->val.ptr = cv_mem ? &cv_mem->cv_mxstep : 0;
		return id;
	}
	if (name ? !strcasecmp(name, "hnilwarns") : (pos == ++id)) {
		if (cv && (id = setMaxHNil(cv, src)) < 0) return id;
		prop->name = "hnilwarns"; prop->desc = "max. warnings for 't + h == t'";
		prop->val.fmt = "i"; prop->val.ptr = cv_mem ? &cv_mem->cv_mxhnil : 0;
		return id;
	}
	if (name ? (!strcasecmp(name, "stepinit") || !strcasecmp(name, "h") || !strcasecmp(name, "hin") || !strcasecmp(name, "h0")) : (pos == ++id)) {
		if (cv && (id = setInitStep(cv, src)) < 0) return id;
		prop->name = "hin"; prop->desc = "initial stepsize";
		prop->val.fmt = "d"; prop->val.ptr = cv_mem ? &cv_mem->cv_hin : 0;
		return id;
	}
	if (name ? (!strcasecmp(name, "hmin") || !strcasecmp(name, "stepmin")) : (pos == ++id)) {
		if (cv && (id = setMinStep(cv, src)) < 0) return id;
		prop->name = "hmin"; prop->desc = "minimal stepsize";
		prop->val.fmt = "d"; prop->val.ptr = cv_mem ? &cv_mem->cv_hmin : 0;
		return id;
	}
	if (name ? (!strcasecmp(name, "hmax") || !strcasecmp(name, "stepmax")) : (pos == ++id)) {
		if (cv && (id = setMaxStep(cv, src)) < 0) return id;
		prop->name = "hmax"; prop->desc = "maximal stepsize";
		prop->val.fmt= ""; prop->val.ptr = 0; /* saved as inverse */
		return id;
	}
	
	errno = EINVAL;
	return MPT_ERROR(BadArgument);
}
