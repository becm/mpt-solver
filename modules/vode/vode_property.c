/*!
 * set vode integrator parameter.
 */

#include <stdlib.h>
#include <strings.h>

#include "version.h"

#include "meta.h"

#include "vode.h"

static int setInt(MPT_SOLVER_STRUCT(vode) *vd, size_t pos, int val)
{
	int *iwk = vd->iwork.iov_base;
	size_t size = vd->iwork.iov_len / sizeof(int);
	if (pos >= size) {
		if (!val) {
			return 0;
		}
		if (!(iwk = mpt_vecpar_alloc(&vd->iwork, pos + 1, sizeof(int)))) {
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
		if (!(rwk = mpt_vecpar_alloc(&vd->rwork, pos + 1, sizeof(double)))) {
			return MPT_ERROR(BadOperation);
		}
	}
	rwk[pos] = val;
	return 1;
}
static int setJacobian(MPT_SOLVER_STRUCT(vode) *vd, MPT_INTERFACE(metatype) *src)
{
	char *key;
	int32_t mu, ml;
	int ret, len;
	
	if (!src) {
		vd->miter = 0;
		return 0;
	}
	if ((ret = src->_vptr->conv(src, 'k' | MPT_ENUM(ValueConsume), &key)) < 0) {
		return ret;
	}
	if (!ret || !key) {
		vd->miter = 0;
		return 0;
	}
	switch (key[0]) {
		case 'n': case 'N': vd->miter = 0; return ret;
		case 'F': vd->miter = 1; return ret;
		case 'f': vd->miter = 2; return ret;
		case 'd': case 'D': vd->miter = 3; return ret;
		case 'B': case 'b': break;
		default:
			return MPT_ERROR(BadValue);
	}
	len = 1;
	if ((ret = src->_vptr->conv(src, 'i' | MPT_ENUM(ValueConsume), &ml)) > 0) {
		++len;
		if ((ret = src->_vptr->conv(src, 'i' | MPT_ENUM(ValueConsume), &mu)) > 0) {
			++len;
		} else {
			mu = ml;
		}
	} else {
		ml = mu = vd->ivp.neqs;
	}
	
	if (setInt(vd, 0, ml) < 0
	    || setInt(vd, 1, mu) < 0) {
		return MPT_ERROR(BadOperation);
	}
	vd->miter = (key[0] == 'B') ? 4 : 5;
	
	return len;
}
static int setStepType(MPT_SOLVER_STRUCT(vode) *vd, MPT_INTERFACE(metatype) *src)
{
	double val;
	char *key;
	int ret;
	
	if (!src) {
		vd->itask = 1;
		return 0;
	}
	if ((ret = src->_vptr->conv(src, 'k', &key)) < 0) {
		return ret;
	}
	if (!ret || !key) {
		vd->itask = 1;
		return 0;
	}
	ret = 1;
	switch (key[0]) {
		/* single step */
		case 's': case '2': vd->itask = 2; return ret;
		/* disable interpolation */
		case 'n': case 'N': case '3': vd->itask = 3; return ret;
		/* enable interpolation */
		case 'i': case '1': vd->itask = 1; return ret;
		/* opteration till tstop */
		case 'I': case 'S': case '4': case '5':
			if ((ret = src->_vptr->conv(src, 'd', &val)) > 0) {
				if (setReal(vd, 0, val) < 0) {
					return MPT_ERROR(BadOperation);
				}
				ret = 2;
				break;
			}
			ret = 1;
		default:
			return MPT_ERROR(BadValue);
	}
	vd->itask = (key[0] == 'S' || key[0] == '5') ? 5 : 4;
	
	return ret;
}
static int setMethod(MPT_SOLVER_STRUCT(vode) *data, MPT_INTERFACE(metatype) *src)
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
		case 'a': data->jsv = -1; data->meth = 1; return 1;
		case 'A': data->jsv =  1; data->meth = 1; return 1;
		case 'b': data->jsv = -1; data->meth = 2; return 1;
		case 'B': data->jsv =  1; data->meth = 2; return 1;
		default: return MPT_ERROR(BadValue);
	}
}

extern int mpt_vode_set(MPT_SOLVER_STRUCT(vode) *vd, const char *name, MPT_INTERFACE(metatype) *src)
{
	int ret = 0;
	
	if (!name) {
		double t = vd->t;
		size_t required;
		
		if (src && (ret = src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), &t) <= 0)) {
			if (ret < 0) {
				return ret;
			}
			src = 0;
		}
		required = vd->ivp.neqs * (vd->ivp.pint + 1);
		if ((ret = mpt_vecpar_set(&vd->y, required, src)) < 0) {
			return ret;
		}
		vd->t = t;
		
		return src ? ++ret : 0;
	}
	else if (!*name) {
		MPT_SOLVER_IVP_STRUCT(parameters) ivp = vd->ivp;
		ret = 0;
		if (src && (ret =  mpt_ivppar_set(&ivp, src)) < 0) {
			return ret;
		}
		mpt_vode_fini(vd);
		mpt_vode_init(vd);
		
		vd->ivp = ivp;
		return ret;
	}
	if (!strcasecmp(name, "atol")) {
		return mpt_vecpar_settol(&vd->atol, src, __MPT_IVP_ATOL);
	}
	if (!strcasecmp(name, "rtol")) {
		return mpt_vecpar_settol(&vd->rtol, src, __MPT_IVP_RTOL);
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
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "mxstep") || !strcasecmp(name, "iwork6")) {
		int32_t val = 0;
		if (src && (ret = src->_vptr->conv(src, 'i', &val)) < 0) return ret;
		if (setInt(vd, 5, val) < 0) return MPT_ERROR(BadOperation);
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "mxhnil") || !strcasecmp(name, "iwork7")) {
		int32_t val = 0;
		if (src && (ret = src->_vptr->conv(src, 'i', &val)) < 0) return ret;
		if (setInt(vd, 6, val) < 0) return MPT_ERROR(BadOperation);
		return ret ? 1 : 0;
	}
	/* real array parameter */
	if (!strcasecmp(name, "h0") || !strcasecmp(name, "rwork5")) {
		double val = 0;
		if (src && (ret = src->_vptr->conv(src, 'd', &val)) < 0) return ret;
		if (setReal(vd, 4, val) < 0) return MPT_ERROR(BadOperation);
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "hmax") || !strcasecmp(name, "rwork6")) {
		double val = 0;
		if (src && (ret = src->_vptr->conv(src, 'd', &val)) < 0) return ret;
		if (setReal(vd, 5, val) < 0) return MPT_ERROR(BadOperation);
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "hmin") || !strcasecmp(name, "rwork7")) {
		double val = 0;
		if (src && (ret = src->_vptr->conv(src, 'd', &val)) < 0) return ret;
		if (setReal(vd, 6, val) < 0) return MPT_ERROR(BadOperation);
		return ret ? 1 : 0;
	}
	return MPT_ERROR(BadArgument);
}
extern int mpt_vode_get(const MPT_SOLVER_STRUCT(vode) *vd, MPT_STRUCT(property) *prop)
{
	const char *name;
	intptr_t pos = -1, id;
	
	if (!prop) {
		return MPT_ENUM(TypeSolver);
	}
	if (!(name = prop->name)) {
		if ((pos = (intptr_t) prop->desc) < 0) {
			return MPT_ERROR(BadArgument);
		}
	}
	else if (!*name) {
		prop->name = "vode"; prop->desc = "implicit ODE solver with BDF";
		prop->val.fmt  = "iid"; prop->val.ptr = &vd->ivp;
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
		if (!vd) { prop->val.fmt = "d"; prop->val.ptr = &vd->atol.d.val; }
		else { id = mpt_vecpar_get(&vd->atol, &prop->val); }
		prop->name = "atol"; prop->desc = "absolute tolerances";
		return id;
	}
	if (name ? !strcasecmp(name, "rtol") : (pos == ++id)) {
		if (!vd) { prop->val.fmt = "d"; prop->val.ptr = &vd->rtol.d.val; }
		else { id = mpt_vecpar_get(&vd->rtol, &prop->val); }
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
