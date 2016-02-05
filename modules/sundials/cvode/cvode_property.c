/*!
 * set CVode integrator parameter
 */

#include <string.h>
#include <strings.h>
#include <errno.h>

#include <cvode/cvode_impl.h>

#include "version.h"

#include "sundials.h"

static const char bdfText[] = "BDF", adamsText[] = "Adams";

static int setMethod(MPT_SOLVER_STRUCT(cvode) *cv, MPT_INTERFACE(metatype) *src)
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

/*!
 * \ingroup mptSundialsCVode
 * \brief set CVode property
 * 
 * Query property of CVode solver
 * 
 * \param cv   CVode data
 * \param name name of property to change
 * \param src  data source to change property
 * 
 * \retval <0   failure
 * \retval >=0  used values
 */
extern int sundials_cvode_set(MPT_SOLVER_STRUCT(cvode) *cv, const char *name, MPT_INTERFACE(metatype) *src)
{
	CVodeMem cv_mem;
	int ret = 0;
	
	if (!cv || !(cv_mem = cv->mem)) {
		return MPT_ERROR(BadArgument);
	}
	if (!name) {
		realtype t = 0;
		long required = cv->ivp.pint + 1;
		/* initialize with zeros */
		if (src && (ret = src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), &t)) <= 0) {
			if (ret < 0) {
				return ret;
			}
			src = 0;
		}
		if ((ret = sundials_vector_set(&cv->sd.y, required * cv->ivp.neqs, src)) < 0) {
			return ret;
		}
		if (src) {
			++ret;
		}
		cv->t = t;
		return ret;
	}
	if (!*name) {
		if (src && (ret = mpt_ivppar_set(&cv->ivp, src)) < 0) {
			return ret;
		}
		sundials_cvode_reset(cv);
		return ret;
	}
	if (!strcasecmp(name, "atol")) {
		return mpt_vecpar_settol(&cv->atol, src, __MPT_IVP_ATOL);
		return ret;
	}
	if (!strcasecmp(name, "rtol")) {
		return mpt_vecpar_settol(&cv->rtol, src, __MPT_IVP_RTOL);
	}
	if (!strncasecmp(name, "jac", 3)) {
		return sundials_jacobian(&cv->sd, cv->ivp.neqs, src);
	}
	if (!strcasecmp(name, "method")) {
		return setMethod(cv, src);
	}
	if (!strcasecmp(name, "maxord")) {
		long val = cv_mem->cv_qmax;
		if (src && (ret = src->_vptr->conv(src, MPT_ENUM(TypeLong), &val)) < 0) return ret;
		else if (CVodeSetMaxOrd(cv_mem, val) < 0) return MPT_ERROR(BadValue);
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "mxstep") || !strcasecmp(name, "maxstep") || !strcasecmp(name, "maxnumsteps")) {
		long val = 0;
		if (src && (ret = src->_vptr->conv(src, MPT_ENUM(TypeLong), &val)) < 0) return ret;
		if (CVodeSetMaxNumSteps(cv_mem, val) < 0) return MPT_ERROR(BadValue);
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "hnilwarns")) {
		long val = 0;
		if (src && (ret = src->_vptr->conv(src, MPT_ENUM(TypeLong), &val)) < 0) return ret;
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
extern int sundials_cvode_get(const MPT_SOLVER_STRUCT(cvode) *cv, MPT_STRUCT(property) *prop)
{
	static const char longfmt[] = { MPT_ENUM(TypeLong), 0 };
	static const char realfmt[] = { MPT_SOLVER_ENUM(SundialsRealtype), 0 };
	static const char dblfmt[] = { 'd', 0 };
	const char *name;
	intptr_t pos = 0, id;
	CVodeMem cv_mem = cv ? cv->mem : 0;
	
	if (!prop) {
		return MPT_ENUM(TypeSolver);
	}
	if (!(name = prop->name)) {
		if ((pos = (intptr_t) prop->desc) < 0) {
			errno = EINVAL;
			return MPT_ERROR(BadArgument);
		}
	}
	else if (!*name) {
		prop->name = "cvode"; prop->desc = "ODE solver from Sundials Library";
		prop->val.fmt = "ii"; prop->val.ptr = &cv->ivp;
		return MPT_SOLVER_ENUM(ODE) | MPT_SOLVER_ENUM(PDE);
	}
	else if (!strcasecmp(name, "version")) {
		static const char version[] = BUILD_VERSION"\0";
		prop->name = "version"; prop->desc = "solver release information";
		prop->val.fmt = 0; prop->val.ptr = version;
		return 0;
	}
	
	id = -1;
	if (name ? !strcasecmp(name, "atol") : (pos == ++id)) {
		if (!cv) { prop->val.fmt = dblfmt; prop->val.ptr = &cv->atol.d.val; }
		else { id = mpt_vecpar_get(&cv->atol, &prop->val); }
		prop->name = "atol"; prop->desc = "absolute tolerances";
		return id;
	}
	if (name ? !strcasecmp(name, "rtol") : (pos == ++id)) {
		if (!cv) { prop->val.fmt = dblfmt; prop->val.ptr = &cv->rtol.d.val; }
		else { id = mpt_vecpar_get(&cv->rtol, &prop->val); };
		prop->name = "rtol"; prop->desc = "relative tolerances";
		return id;
	}
	if (name ? !strncasecmp(name, "jac", 3) : (pos == ++id)) {
		prop->name = "jacobian"; prop->desc = "jacobian type";
		prop->val.fmt = "y"; prop->val.ptr = &cv->sd.jacobian;
		return cv ? (cv->sd.jacobian ? 1 : 0) : id;
	}
	if (name ? !strcasecmp(name, "method") : (pos == ++id)) {
		prop->name = "method"; prop->desc = "solver method";
		prop->val.fmt = prop->val.ptr = 0;
		if (!cv_mem) return id;
		switch (cv_mem->cv_lmm) {
		  case CV_BDF: prop->val.ptr = bdfText; break;
		  case CV_ADAMS: prop->val.ptr = adamsText; break;
		  default: prop->val.ptr = "";
		}
		return id;
	}
	if (name ? !strcasecmp(name, "maxord") : (pos == ++id)) {
		prop->name = "maxord"; prop->desc = "maximum order of steps";
		prop->val.fmt = "i"; prop->val.ptr = 0;
		if (!cv_mem) return id;
		prop->val.ptr = &cv_mem->cv_qmax;
		return cv_mem->cv_qmax == ADAMS_Q_MAX ? 0 : 1;
	}
	if (name ? (!strcasecmp(name, "mxstep") || !strcasecmp(name, "maxstep") || !strcasecmp(name, "maxnumsteps")) : (pos == ++id)) {
		prop->name = "mxstep"; prop->desc = "allowed function evaluations per call";
		prop->val.fmt = longfmt; prop->val.ptr = 0;
		if (!cv_mem) return id;
		prop->val.ptr = &cv_mem->cv_mxstep;
		return cv_mem->cv_mxstep ? 1 : 0;
	}
	if (name ? !strcasecmp(name, "hnilwarns") : (pos == ++id)) {
		prop->name = "hnilwarns"; prop->desc = "max. warnings for 't + h == t'";
		prop->val.fmt = "i"; prop->val.ptr = 0;
		if (!cv_mem) return id;
		prop->val.ptr = &cv_mem->cv_mxhnil;
		return cv_mem->cv_mxhnil ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "stepinit") || !strcasecmp(name, "h") || !strcasecmp(name, "hin") || !strcasecmp(name, "h0")) : (pos == ++id)) {
		prop->name = "hin"; prop->desc = "initial stepsize";
		prop->val.fmt = "d"; prop->val.ptr = 0;
		if (!cv_mem) return id;
		prop->val.ptr = &cv_mem->cv_hin;
		return cv_mem->cv_hin ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "hmin") || !strcasecmp(name, "stepmin")) : (pos == ++id)) {
		prop->name = "hmin"; prop->desc = "minimal stepsize";
		prop->val.fmt = "d"; prop->val.ptr = 0;
		if (!cv_mem) return id;
		prop->val.ptr = &cv_mem->cv_hmin;
		return cv_mem->cv_hmin ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "hmax") || !strcasecmp(name, "stepmax")) : (pos == ++id)) {
		prop->name = "hmax"; prop->desc = "maximal stepsize";
		prop->val.fmt = realfmt; prop->val.ptr = 0;
		if (!cv) return id;
		prop->val.ptr = &cv->hmax;
		return cv->hmax ? 1 : 0;
	}
	return MPT_ERROR(BadArgument);
}
