/*!
 * set dDASSL integrator parameters
 */

#include <string.h>
#include <strings.h>
#include <errno.h>

#include "release.h"
#include "version.h"

#include "dassl.h"

static int setIvp(MPT_SOLVER_STRUCT(dassl) *data, MPT_INTERFACE(source) *src)
{
	if (!src) {
		return mpt_ivppar_set(&data->ivp, src);
	} else {
		MPT_SOLVER_STRUCT(ivppar) ivp = data->ivp;
		int ret;
		
		if ((ret =  mpt_ivppar_set(&ivp, src)) < 0) {
			return ret;
		}
		mpt_dassl_fini(data);
		if (mpt_dassl_init(data) < 0) {
			return -1;
		}
		data->ivp = ivp;
		
		((double *) data->rwork.iov_base)[3] = ivp.last;
		
		return ret;
	}
}
static int setJacobian(MPT_SOLVER_STRUCT(dassl) *data, MPT_INTERFACE(source) *src)
{
	char *key;
	int *iwork, len;
	
	if (!src) return data->info[4] + data->info[5] * 2;
	
	if ((len = src->_vptr->conv(src, 'k', &key)) < 0) {
		return len;
	}
	if (!len || !key) {
		data->info[4] = 1;
		data->info[5] = 0;
		return 0;
	}
	
	switch (key[0]) {
		case 'F': data->info[4] = 1; data->info[5] = 0; return 0;
		case 'f': data->info[4] = 0; data->info[5] = 0; return 0;
		case 'B': data->info[4] = 1; data->info[5] = 1; break;
		case 'b': data->info[4] = 0; data->info[5] = 1; break;
		default:
			errno = EINVAL;
			return MPT_ERROR(BadValue);
	}
	iwork = data->iwork.iov_base;
	
	if (src->_vptr->conv(src, 'i', iwork) <= 0) {
		iwork[1] = iwork[0] = data->ivp.neqs; return 1;
	}
	if (src->_vptr->conv(src, 'i', iwork+1) <= 0) {
		iwork[1] = iwork[0]; return 2;
	}
	return 3;
}
static int setInfo3(MPT_SOLVER_STRUCT(dassl) *data, MPT_INTERFACE(source) *src)
{
	int len;
	
	if (!src) return data->info[2];
	
	if ((len = src->_vptr->conv(src, 'i', data->info+2)) < 0) return len;
	if (!len) data->info[2] = 0;
	return len;
}
static int setTStop(MPT_SOLVER_STRUCT(dassl) *data, MPT_INTERFACE(source) *src)
{
	int len;
	
	if (!src) return data->info[3];
	
	if ((len = src->_vptr->conv(src, 'd', data->rwork.iov_base)) < 0) return len;
	data->info[3] = len ? 1 : 0;
	return len;
}
static int setHMax(MPT_SOLVER_STRUCT(dassl) *data, MPT_INTERFACE(source) *src)
{
	int len;
	
	if (!src) return data->info[6];
	
	if ((len = src->_vptr->conv(src, 'd', ((double *) data->rwork.iov_base) + 1)) < 0) return len;
	data->info[3] = len ? 1 : 0;
	return len;
}
static int setH0(MPT_SOLVER_STRUCT(dassl) *data, MPT_INTERFACE(source) *src)
{
	int len;
	
	if (!src) return data->info[7];
	
	if ((len = src->_vptr->conv(src, 'd', ((double *) data->rwork.iov_base) + 2)) < 0) return len;
	data->info[7] = len ? 1 : 0;
	return len;
}
static int setMaxOrd(MPT_SOLVER_STRUCT(dassl) *data, MPT_INTERFACE(source) *src)
{
	int len;
	
	if (!src) return data->info[8];
	
	if ((len = src->_vptr->conv(src, 'i', data->info+8)) < 0) return len;
	if (!len) data->info[8] = 0;
	return len;
}
static int setInfo10(MPT_SOLVER_STRUCT(dassl) *data, MPT_INTERFACE(source) *src)
{
	int len;
	
	if (!src) return data->info[9];
	
	if ((len = src->_vptr->conv(src, 'i', data->info+9)) < 0) return len;
	if (!len) data->info[9] = 0;
	return len;
}
static int setInfo11(MPT_SOLVER_STRUCT(dassl) *data, MPT_INTERFACE(source) *src)
{
	int len;
	
	if (!src) return data->info[2];
	
	if ((len = src->_vptr->conv(src, 'i', data->info+10)) < 0) return len;
	if (!len) data->info[10] = 0;
	return len;
}
static int setYP(MPT_SOLVER_STRUCT(dassl) *data, MPT_INTERFACE(source) *src)
{
	double *yp;
	int total, curr, len;
	
	if (!src) return data->yp.iov_len / sizeof(double);
	
	if ((len = data->ivp.neqs * (data->ivp.pint + 1)) < 0) {
		errno = EOVERFLOW;
		return MPT_ERROR(BadValue);
	}
	if (!(yp = mpt_vecpar_alloc(&data->yp, len, sizeof(*yp)))) {
		return MPT_ERROR(BadOperation);
	}
	total = 0;
	while ((curr = src->_vptr->conv(src, 'd', yp)) >= 0) {
		total += curr; yp++; if (!--len) return total;
	}
	data->yp.iov_len -= len * sizeof(double);
	
	return total;
}

extern int mpt_dassl_property(MPT_SOLVER_STRUCT(dassl) *data, MPT_STRUCT(property) *prop, MPT_INTERFACE(source) *src)
{
	const char *name;
	intptr_t pos = -1, id;
	
	if (!prop) return (src && data) ? setIvp(data, src) : MPT_ENUM(TypeSolver);
	
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
		id = MPT_SOLVER_ENUM(ODE) | MPT_SOLVER_ENUM(DAE) | MPT_SOLVER_ENUM(PDE);
		if (data && src && (id = setIvp(data, src)) < 0) return id;
		prop->name = "dassl"; prop->desc = "implicit DAE solver with BDF";
		prop->val.fmt = "iid"; prop->val.ptr = &data->ivp;
		return id;
	}
	if (name && !strcasecmp(name, "version")) {
		static const char version[] = MPT_VERSION"\0";
		prop->name = "version"; prop->desc = "solver release information";
		prop->val.fmt = 0; prop->val.ptr = version;
		return 0;
	}
	
	id = -1;
	if (name ? !strcasecmp(name, "atol") : pos == ++id) {
		if (data && (id = mpt_vecpar_value(&data->atol, &prop->val, src)) < 0) return id;
		prop->name = "atol"; prop->desc = "absolute tolerances";
		return id;
	}
	if (name ? !strcasecmp(name, "rtol") : pos == ++id) {
		if (data && (id = mpt_vecpar_value(&data->rtol, &prop->val, src)) < 0) return id;
		prop->name = "rtol"; prop->desc = "relative tolerances";
		return id;
	}
	if (name ? !strncasecmp(name, "jac", 3) : pos == ++id) {
		if (data && (id = setJacobian(data, src)) < 0) return id;
		prop->name = "jacobian"; prop->desc = "(user) jacobian parameters";
		prop->val.fmt = "ii"; prop->val.ptr = data ? data->info+4 : 0;
		return id;
	}
	if (name ? !strcasecmp(name, "info3") : pos == ++id) {
		if (data && (id = setInfo3(data, src)) < 0) return id;
		prop->name = "info3"; prop->desc = "output only at tout";
		prop->val.fmt = "i"; prop->val.ptr = data ? data->info+2 : 0;
		return id;
	}
	if (name ? (!strcasecmp(name, "info4") || !strcasecmp(name, "tstop")) : pos == ++id) {
		if (data && (id = setTStop(data, src)) < 0) return id;
		prop->name = "tstop"; prop->desc = "do not step past 'tstop'";
		prop->val.fmt = "d"; prop->val.ptr = data ? data->rwork.iov_base : 0;
		return id;
	}
	if (name ? (!strcasecmp(name, "info7") || !strcasecmp(name, "hmax")) : pos == ++id) {
		if (data && (id = setHMax(data, src)) < 0) return id;
		prop->name = "hmax"; prop->desc = "maximum step size";
		prop->val.fmt = "d"; prop->val.ptr = (data && data->rwork.iov_base) ? ((double *) data->rwork.iov_base) + 1 : 0;
		return id;
	}
	if (name ? (!strcasecmp(name, "info8") || !strcasecmp(name, "h0") || !strcasecmp(name, "initstep")) : pos == ++id) {
		if (data && (id = setH0(data, src)) < 0) return id;
		prop->name = "h0"; prop->desc = "explicit initial step size";
		prop->val.fmt = "d"; prop->val.ptr = (data && data->rwork.iov_base) ? ((double *) data->rwork.iov_base) + 2 : 0;
		return id;
	}
	if (name ? (!strcasecmp(name, "info9") || !strcasecmp(name, "maxord")) : pos == ++id) {
		if (data && (id = setMaxOrd(data, src)) < 0) return id;
		prop->name = "maxord"; prop->desc = "maximum order";
		prop->val.fmt = "i"; prop->val.ptr = data ? data->info + 8 : 0;
		return id;
	}
	if (name ? !strcasecmp(name, "info10") : pos == ++id) {
		if (data && (id = setInfo10(data, src)) < 0) return id;
		prop->name = "info10"; prop->desc = "restrict to nonnegative solutions";
		prop->val.fmt = "i"; prop->val.ptr = data ? data->info + 9 : 0;
		return id;
	}
	if (name ? !strcasecmp(name, "info11") : pos == ++id) {
		if (data && (id = setInfo11(data, src)) < 0) return id;
		prop->name = "info11"; prop->desc = "consistent initial values";
		prop->val.fmt = "i"; prop->val.ptr = data ? data->info + 10 : 0;
		return id;
	}
	if (name ? !strncasecmp(name, "yp", 2) : pos == ++id) {
		static const char fmt[] = { 'd' | (char) MPT_ENUM(TypeVector) };
		if (data && (id = setYP(data, src))) return id;
		prop->name = "yprime"; prop->desc = "deviation at current time";
		prop->val.fmt = fmt; prop->val.ptr = data ? &data->yp : 0;
		return id;
	}
	
	errno = EINVAL;
	return MPT_ERROR(BadArgument);
}
