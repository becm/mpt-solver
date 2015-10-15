/*!
 * set MINPACK solver parameter
 */

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <float.h>
#include <errno.h>
#include <ctype.h>

#include "version.h"

#include "minpack.h"

static int setNls(MPT_SOLVER_STRUCT(minpack) *data, MPT_INTERFACE(source) *src)
{
	if (!src) {
		return mpt_nlspar_set(&data->nls, src);
	}
	else {
		MPT_SOLVER_STRUCT(nlspar) nls = data->nls;
		int ret;
		
		if ((ret =  mpt_nlspar_set(&nls, src)) < 0) {
			return ret;
		}
		mpt_minpack_fini(data);
		mpt_minpack_init(data);
		data->nls = nls;
		return ret;
	}
}
static int setXTol(MPT_SOLVER_STRUCT(minpack) *data, MPT_INTERFACE(source) *src)
{
	int len;
	if (!src) return data->xtol ? 1 : 0;
	if (!(len = src->_vptr->conv(src, 'd', &data->xtol))) data->xtol = 0.0;
	return len;
}
static int setFTol(MPT_SOLVER_STRUCT(minpack) *data, MPT_INTERFACE(source) *src)
{
	int len;
	if (!src) return (data->ftol != 1e-7) ? 1 : 0;
	if (!(len = src->_vptr->conv(src, 'd', &data->ftol))) data->xtol = 1e-7;
	return len;
}
static int setGTol(MPT_SOLVER_STRUCT(minpack) *data, MPT_INTERFACE(source) *src)
{
	int len;
	if (!src) return data->gtol ? 1 : 0;
	if (!(len = src->_vptr->conv(src, 'd', &data->gtol))) data->gtol = 0.0;
	return len;
}
static int setJacobian(MPT_SOLVER_STRUCT(minpack) *data, MPT_INTERFACE(source) *src)
{
	char *key;
	int l1, l2, l3;
	
	if (!src) return (data->mu || data->ml) ? 0 : 1;
	
	if ((l1 = src->_vptr->conv(src, 'k', &key)) < 0) return l1;
	
	if (!l1 || !key) return data->mu = data->ml = 0;
	
	switch (key[0]) {
		case 'F': data->mu = data->ml = 0; return l1;
		case 'f': data->mu = data->ml = 0; return l1;
		case 'B': case 'b': break;
		default:
			errno = EINVAL;
			return MPT_ERROR(BadValue);
	}
	l3 = 0;
	
	if ((l2 = src->_vptr->conv(src, 'i', &data->ml)) <= 0) {
		l2 = 0; data->mu = data->ml = 0;
	}
	else if ((l3 = src->_vptr->conv(src, 'i', &data->mu)) <= 0) {
		l3 = 0; data->mu = data->ml;
	}
	return l1 + l2 + l3;
}
static int setMaxFev(MPT_SOLVER_STRUCT(minpack) *data, MPT_INTERFACE(source) *src)
{
	int len;
	if (!src) return data->maxfev;
	if (!(len = src->_vptr->conv(src, 'i', &data->maxfev))) data->maxfev = 0;
	return len;
}
static int setFactor(MPT_SOLVER_STRUCT(minpack) *data, MPT_INTERFACE(source) *src)
{
	int len;
	if (!src) return (data->factor == 200.) ? 0 : 1;
	if (!(len = src->_vptr->conv(src, 'd', &data->factor))) data->factor = 200.0;
	return len;
}
static int setEpsFcn(MPT_SOLVER_STRUCT(minpack) *data, MPT_INTERFACE(source) *src)
{
	int len;
	if (!src) return data->epsfcn ? 1 : 0;
	if (!(len = src->_vptr->conv(src, 'd', &data->epsfcn))) data->epsfcn = 0.0;
	return len;
}
static int setNPrint(MPT_SOLVER_STRUCT(minpack) *data, MPT_INTERFACE(source) *src)
{
	int len;
	if (!src) return data->nprint;
	if (!(len = src->_vptr->conv(src, 'c', &data->nprint))) data->nprint = 0;
	return len;
}
static int setDiag(MPT_SOLVER_STRUCT(minpack) *data, MPT_INTERFACE(source) *src)
{
	double *diag;
	int total = 0, curr, nd;
	
	if (!src) return data->diag.iov_len / sizeof(double);
	
	nd = data->nls.nval;
	if (!(diag = mpt_vecpar_alloc(&data->diag, nd, sizeof(*diag)))) {
		return MPT_ERROR(BadOperation);
	}
	while ((curr = src->_vptr->conv(src, 'd', diag)) > 0) {
		total += curr; diag++; if ( !--nd ) return total;
	}
	data->diag.iov_len -= nd * sizeof(double);
	
	return total;
}

extern int mpt_minpack_property(MPT_SOLVER_STRUCT(minpack) *data, MPT_STRUCT(property) *prop, MPT_INTERFACE(source) *src)
{
	const char *name;
	intptr_t pos = -1, id;
	
	if (!prop) return (src && data) ? setNls(data, src) : MPT_ENUM(TypeSolver);
	
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
		id = MPT_SOLVER_ENUM(NlsVector) | MPT_SOLVER_ENUM(NlsOverDet);
		if (data && src && (id = setNls(data, src)) < 0) return id;
		prop->name = "minpack"; prop->desc = "solver for (overdetermined) nonlinear equotations";
		prop->val.fmt = "ii"; prop->val.ptr = &data->nls;
		return id;
	}
	if (name && !strcasecmp(name, "version")) {
		static const char version[] = BUILD_VERSION"\0";
		prop->name = "version"; prop->desc = "solver release information";
		prop->val.fmt = 0; prop->val.ptr = version;
		return 0;
	}
	
	id = -1;
	if (name ? !strcasecmp(name, "xtol") : (pos == ++id)) {
		if (data && (id = setXTol(data, src)) < 0) return id;
		prop->name = "xtol"; prop->desc = "desired relative error";
		prop->val.fmt = "d"; prop->val.ptr = &data->xtol;
		return id;
	}
	if (name ? !strcasecmp(name, "ftol") : (pos == ++id)) {
		if (data && (id = setFTol(data, src)) < 0) return id;
		prop->name = "ftol"; prop->desc = "desired residual";
		prop->val.fmt = "d"; prop->val.ptr = &data->ftol;
		return id;
	}
	if (name ? !strcasecmp(name, "gtol") : (pos == ++id)) {
		if (data && (id = setGTol(data, src)) < 0) return id;
		prop->name = "gtol"; prop->desc = "desired orthogonality";
		prop->val.fmt = "d"; prop->val.ptr = &data->gtol; prop->val.fmt= "d";
		return id;
	}
	if (name ? !strncasecmp(name, "jac", 3) : (pos == ++id)) {
		if (data && (id = setJacobian(data, src)) < 0) return id;
		prop->name = "jacobian"; prop->desc = "(user) jacobian settings";
		prop->val.fmt = "ii"; prop->val.ptr = &data->mu;
		return id;
	}
	if (name ? !strcasecmp(name, "maxfev") : (pos == ++id)) {
		if (data && (id = setMaxFev(data, src)) < 0) return id;
		prop->name = "maxfev"; prop->desc = "max. function evaluations per call";
		prop->val.fmt = "i"; prop->val.ptr = &data->maxfev;
		return id;
	}
	if (name ? !strncasecmp(name, "fac", 3) : (pos == ++id)) {
		if (data && (id = setFactor(data, src)) < 0) return id;
		prop->name = "factor"; prop->desc = "initial step bound";
		prop->val.fmt = "d"; prop->val.ptr = &data->factor;
		return id;
	}
	if (name ? !strncasecmp(name, "eps", 3) : (pos == ++id)) {
		if (data && (id = setEpsFcn(data, src)) < 0) return id;
		prop->name = "epsfcn"; prop->desc = "initial forward-difference step length";
		prop->val.fmt = "d"; prop->val.ptr = &data->epsfcn; prop->val.fmt= "d";
		return id;
	}
	if (name ? !strncasecmp(name, "eps", 3) : (pos == ++id)) {
		if (data && (id = setNPrint(data, src)) < 0) return id;
		prop->name = "nprint"; prop->desc = "iteration output";
		prop->val.fmt = "c"; prop->val.ptr = &data->nprint;
		return id;
	}
	if (name ? !strcasecmp(name, "diag") : (pos == ++id)) {
		static const char fmt[] = { 'd' | (char) MPT_ENUM(TypeVector) };
		if (data && (id = setDiag(data, src)) < 0) return id;
		prop->name = "diag"; prop->desc = "scale factor for variables";
		prop->val.fmt = fmt; prop->val.ptr = &data->diag;
		return id;
	}
	
	errno = EINVAL;
	return MPT_ERROR(BadArgument);
}
