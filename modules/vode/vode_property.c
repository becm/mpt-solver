/*!
 * set vode integrator parameter.
 */

#include <stdlib.h>
#include <strings.h>

#include "version.h"
#include "module.h"

#include "vode.h"

#include "module_functions.h"

static int setInt(MPT_SOLVER_STRUCT(vode) *vd, size_t pos, int val)
{
	int *iwk = vd->iwork.iov_base;
	size_t size = vd->iwork.iov_len / sizeof(int);
	if (pos >= size) {
		if (!val) {
			return 0;
		}
		if (!(iwk = mpt_solver_module_valloc(&vd->iwork, pos + 1, sizeof(int)))) {
			return MPT_ERROR(BadOperation);
		}
	}
	iwk[pos] = val;
	return 1;
}
static int setReal(MPT_SOLVER_STRUCT(vode) *vd, size_t pos, double val)
{
	double *rwk = vd->rwork.iov_base;
	size_t size = vd->rwork.iov_len / sizeof(double);
	if (pos >= size) {
		if (!val) {
			return 0;
		}
		if (!(rwk = mpt_solver_module_valloc(&vd->rwork, pos + 1, sizeof(double)))) {
			return MPT_ERROR(BadOperation);
		}
	}
	rwk[pos] = val;
	return 1;
}
static int setJacobian(MPT_SOLVER_STRUCT(vode) *vd, const MPT_INTERFACE(metatype) *src)
{
	MPT_STRUCT(module_value) val = MPT_MODULE_VALUE_INIT;
	const char *key;
	int32_t ml, mu;
	long max;
	int ret;
	char mode;
	
	if (!src) {
		vd->miter = 0;
		return 0;
	}
	key = 0;
	if ((ret = mpt_module_value_init(&val, src)) < 0) {
		if ((ret = src->_vptr->conv(src, 'k', &key)) < 0) {
			return ret;
		}
		ret = 0;
	}
	else if ((ret = mpt_module_value_key(&val, &key)) < 0) {
		return ret;
	} else {
		ret = 1;
	}
	if (!key || !(mode = *key)) {
		vd->miter = 0;
		return 0;
	}
	switch (mode) {
		case 'n': case 'N': vd->miter = 0; return ret;
		case 'F': vd->miter = 1; return ret;
		case 'f': vd->miter = 2; return ret;
		case 'd': case 'D': vd->miter = 3; return ret;
		case 'B': case 'b': break;
		default:
			return MPT_ERROR(BadValue);
	}
	if ((ret = mpt_module_value_int(&val, &ml)) < 0) {
		return ret;
	}
	else if (!ret) {
		ml = mu = vd->ivp.neqs;
		ret = 1;
	}
	else if ((ret = mpt_module_value_int(&val, &mu)) < 0) {
		return ret;
	}
	else if (!ret) {
		mu = ml;
		ret = 2;
	}
	else {
		ret = 3;
	}
	if (ml < 0 || mu < 0) {
		return MPT_ERROR(BadValue);
	}
	max = vd->ivp.neqs * (vd->ivp.pint + 1);
	if (ml >= max || mu >= max) {
		return MPT_ERROR(BadValue);
	}
	if (setInt(vd, 0, ml) < 0
	 || setInt(vd, 1, mu) < 0) {
		return MPT_ERROR(BadOperation);
	}
	vd->miter = (mode == 'B') ? 4 : 5;
	
	return ret;
}
static int setStepType(MPT_SOLVER_STRUCT(vode) *vd, const MPT_INTERFACE(metatype) *src)
{
	MPT_STRUCT(module_value) val;
	const char *key;
	double tcrit;
	int ret;
	char mode;
	
	if (!src) {
		vd->itask = 1;
		return 0;
	}
	key = 0;
	if ((ret = mpt_module_value_init(&val, src)) < 0) {
		if ((ret = src->_vptr->conv(src, 'k', &key)) < 0) {
			return ret;
		}
		ret = 0;
	}
	else if ((ret = mpt_module_value_key(&val, &key)) < 0) {
		return ret;
	} else {
		ret = 1;
	}
	if (!key || !(mode = *key)) {
		vd->itask = 1;
		return 0;
	}
	switch (mode) {
		/* single step */
		case 's': case '2': vd->itask = 2; return ret;
		/* disable interpolation */
		case 'n': case 'N': case '3': vd->itask = 3; return ret;
		/* enable interpolation */
		case 'i': case '1': vd->itask = 1; return ret;
		/* opteration till tstop */
		case 'I': case 'S': case '4': case '5':
			break;
		default:
			return MPT_ERROR(BadValue);
	}
	if (!ret || !(ret = mpt_module_value_double(&val, &tcrit))) {
		return MPT_ERROR(MissingData);
	}
	if (ret < 0) {
		return ret;
	}
	if (setReal(vd, 0, tcrit) < 0) {
		return MPT_ERROR(BadOperation);
	}
	vd->itask = (mode == 'S' || mode == '5') ? 5 : 4;
	
	return 2;
}
static int setMethod(MPT_SOLVER_STRUCT(vode) *data, const MPT_INTERFACE(metatype) *src)
{
	char *key;
	int ret;
	
	if (!src) {
		data->meth = 1;
		return 0;
	}
	if ((ret = src->_vptr->conv(src, 'k', &key)) < 0) {
		return ret;
	}
	if (!ret || !key) {
		data->meth = 1;
		return 0;
	}
	switch (key[0]) {
		case 'a': data->jsv = -1; data->meth = 1; return 0;
		case 'A': data->jsv =  1; data->meth = 1; return 0;
		case 'b': data->jsv = -1; data->meth = 2; return 0;
		case 'B': data->jsv =  1; data->meth = 2; return 0;
		default: return MPT_ERROR(BadValue);
	}
}

extern int mpt_vode_set(MPT_SOLVER_STRUCT(vode) *vd, const char *name, const MPT_INTERFACE(metatype) *src)
{
	int ret = 0;
	
	if (!name) {
		return MPT_SOLVER_MODULE_FCN(ivp_state)(&vd->ivp, &vd->t, &vd->y, src);
	}
	else if (!*name) {
		MPT_IVP_STRUCT(parameters) ivp = MPT_IVPPAR_INIT;
		ret = 0;
		if (src && (ret =  mpt_solver_module_ivpset(&ivp, src)) < 0) {
			return ret;
		}
		mpt_vode_fini(vd);
		mpt_vode_init(vd);
		vd->ivp = ivp;
		
		return ret;
	}
	if (!strcasecmp(name, "atol")) {
		return mpt_solver_module_tol_set(&vd->atol, src, __MPT_IVP_ATOL);
	}
	if (!strcasecmp(name, "rtol")) {
		return mpt_solver_module_tol_set(&vd->rtol, src, __MPT_IVP_RTOL);
	}
	if (!strncasecmp(name, "jac", 3)) {
		return setJacobian(vd, src);
	}
	if (!strncasecmp(name, "itask", 2)) {
		return setStepType(vd, src);
	}
	if (!strncasecmp(name, "method", 4)) {
		return setMethod(vd, src);
	}
	/* integer array parameter */
	if (!strcasecmp(name, "maxord") || !strcasecmp(name, "iwork5")) {
		int32_t val = 0;
		if (src && (ret = src->_vptr->conv(src, 'i', &val)) < 0) return ret;
		if (setInt(vd, 4, val) < 0) return MPT_ERROR(BadOperation);
		return 0;
	}
	if (!strcasecmp(name, "mxstep") || !strcasecmp(name, "iwork6")) {
		int32_t val = 0;
		if (src && (ret = src->_vptr->conv(src, 'i', &val)) < 0) return ret;
		if (setInt(vd, 5, val) < 0) return MPT_ERROR(BadOperation);
		return 0;
	}
	if (!strcasecmp(name, "mxhnil") || !strcasecmp(name, "iwork7")) {
		int32_t val = 0;
		if (src && (ret = src->_vptr->conv(src, 'i', &val)) < 0) return ret;
		if (setInt(vd, 6, val) < 0) return MPT_ERROR(BadOperation);
		return 0;
	}
	/* real array parameter */
	if (!strcasecmp(name, "h0") || !strcasecmp(name, "rwork5")) {
		double val = 0;
		if (src && (ret = src->_vptr->conv(src, 'd', &val)) < 0) return ret;
		if (setReal(vd, 4, val) < 0) return MPT_ERROR(BadOperation);
		return 0;
	}
	if (!strcasecmp(name, "hmax") || !strcasecmp(name, "rwork6")) {
		double val = 0;
		if (src && (ret = src->_vptr->conv(src, 'd', &val)) < 0) return ret;
		if (setReal(vd, 5, val) < 0) return MPT_ERROR(BadOperation);
		return 0;
	}
	if (!strcasecmp(name, "hmin") || !strcasecmp(name, "rwork7")) {
		double val = 0;
		if (src && (ret = src->_vptr->conv(src, 'd', &val)) < 0) return ret;
		if (setReal(vd, 6, val) < 0) return MPT_ERROR(BadOperation);
		return 0;
	}
	return MPT_ERROR(BadArgument);
}
extern int mpt_vode_get(const MPT_SOLVER_STRUCT(vode) *vd, MPT_STRUCT(property) *prop)
{
	const char *name;
	intptr_t pos = -1, id;
	
	if (!prop) {
		return MPT_SOLVER_ENUM(ODE) | MPT_SOLVER_ENUM(PDE);
	}
	if (!(name = prop->name)) {
		if ((pos = (intptr_t) prop->desc) < 0) {
			return MPT_ERROR(BadArgument);
		}
	}
	else if (!*name) {
		prop->name = "vode"; prop->desc = "implicit ODE solver with BDF";
		prop->val.fmt  = "iu"; prop->val.ptr = &vd->ivp;
		return vd->ivp.neqs == 1 && !vd->ivp.pint ? 0 : 1;
	}
	else if (!strcasecmp(name, "version")) {
		static const char version[] = BUILD_VERSION"\0";
		prop->name = "version"; prop->desc = "solver release information";
		prop->val.fmt = 0; prop->val.ptr = version;
		return 0;
	}
	
	id = -1;
	if (name ? !strcasecmp(name, "atol") : (pos == ++id)) {
		if (!vd) { prop->val.fmt = "d"; prop->val.ptr = &vd->atol.d.val; }
		else { id = mpt_solver_module_tol_get(&vd->atol, &prop->val); }
		prop->name = "atol"; prop->desc = "absolute tolerances";
		return id;
	}
	if (name ? !strcasecmp(name, "rtol") : (pos == ++id)) {
		if (!vd) { prop->val.fmt = "d"; prop->val.ptr = &vd->rtol.d.val; }
		else { id = mpt_solver_module_tol_get(&vd->rtol, &prop->val); }
		prop->name = "rtol"; prop->desc = "relative tolerances";
		return id;
	}
	if (name ? !strncasecmp(name, "jac", 3) : (pos == ++id)) {
		prop->name = "jacobian"; prop->desc = "(user) jacobian parameters";
		prop->val.fmt  = "b"; prop->val.ptr = &vd->miter;
		if (!vd) return id;
		return vd->miter ? 1 : 0;
	}
	if (name ? !strncasecmp(name, "itask", 2) : (pos == ++id)) {
		prop->name = "itask"; prop->desc = "step control";
		prop->val.fmt  = "n"; prop->val.ptr = &vd->itask;
		if (!vd) return id;
		return (vd->itask != 1) ? 1 : 0;
	}
	if (name ? !strncasecmp(name, "method", 4) : (pos == ++id)) {
		prop->name = "method"; prop->desc = "iteration method";
		prop->val.fmt  = "b"; prop->val.ptr = &vd->meth;
		if (!vd) return id;
		return (vd->meth != 1) ? 1 : 0;
	}
	/* integer array parameter */
	if (name ? (!strcasecmp(name, "maxord") || !strcasecmp(name, "iwork5")) : (pos == ++id)) {
		prop->name = "maxord"; prop->desc = "maximum order to be allowed";
		prop->val.fmt  = "i"; prop->val.ptr = 0;
		if (!vd) return id;
		if (vd->iwork.iov_len >= 5 * sizeof(int)) prop->val.ptr = ((int *) vd->iwork.iov_base) + 4;
		return prop->val.ptr && *((int *) prop->val.ptr) ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "mxstep") || !strcasecmp(name, "iwork6")) : (pos == ++id)) {
		prop->name = "mxstep"; prop->desc = "max. internal steps per call";
		prop->val.fmt  = "i"; prop->val.ptr = 0;
		if (!vd) return id;
		if (vd->iwork.iov_len >= 6 * sizeof(int)) prop->val.ptr = ((int *) vd->iwork.iov_base) + 5;
		return prop->val.ptr && *((int *) prop->val.ptr) ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "mxhnil") || !strcasecmp(name, "iwork7")) : (pos == ++id)) {
		prop->name = "mxhnil"; prop->desc = "max. warnings for 't + h = t'";
		prop->val.fmt  = "i"; prop->val.ptr = 0;
		if (!vd) return id;
		if (vd->iwork.iov_len >= 7 * sizeof(int)) prop->val.ptr = ((int *) vd->iwork.iov_base) + 6;
		return prop->val.ptr && *((int *) prop->val.ptr) ? 1 : 0;
	}
	/* real array parameter */
	if (name ? (!strcasecmp(name, "h0") || !strcasecmp(name, "rwork5")) : (pos == ++id)) {
		prop->name = "h0"; prop->desc = "explicit initial steps size";
		prop->val.fmt  = "d"; prop->val.ptr = 0;
		if (!vd) return id;
		if (vd->iwork.iov_len >= 5 * sizeof(double)) prop->val.ptr = ((double *) vd->rwork.iov_base) + 4;
		return prop->val.ptr && *((double *) prop->val.ptr) ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "hmax") || !strcasecmp(name, "rwork6")) : (pos == ++id)) {
		prop->name = "hmax"; prop->desc = "maximal internal steps size";
		prop->val.fmt = "d"; prop->val.ptr = 0;
		if (!vd) return id;
		if (vd->iwork.iov_len >= 6 * sizeof(double)) prop->val.ptr = ((double *) vd->rwork.iov_base) + 5;
		return prop->val.ptr && *((double *) prop->val.ptr) ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "hmin") || !strcasecmp(name, "rwork7")) : (pos == ++id)) {
		prop->name = "hmin"; prop->desc = "maximal internal steps size";
		prop->val.fmt  = "d"; prop->val.ptr = 0;
		if (!vd) return id;
		if (vd->iwork.iov_len >= 7 * sizeof(double)) prop->val.ptr = ((double *) vd->rwork.iov_base) + 6;
		return prop->val.ptr && *((double *) prop->val.ptr) ? 1 : 0;
	}
	return MPT_ERROR(BadArgument);
}
