/*!
 * set MEBDFI integrator parameter
 */

#include <string.h>
#include <strings.h>
#include <errno.h>

#include "release.h"
#include "version.h"

#include "mebdfi.h"

static int setIvp(MPT_SOLVER_STRUCT(mebdfi) *data, MPT_INTERFACE(source) *src)
{
	if (!src) {
		return mpt_ivppar_set(&data->ivp, src);
	}
	else {
		MPT_SOLVER_STRUCT(ivppar) ivp = data->ivp;
		int ret;
		
		if ((ret =  mpt_ivppar_set(&ivp, src)) < 0) {
			return ret;
		}
		mpt_mebdfi_fini(data);
		mpt_mebdfi_init(data);
		data->ivp = ivp;
		return ret;
	}
}
static int setJacobian(MPT_SOLVER_STRUCT(mebdfi) *data, MPT_INTERFACE(source) *src)
{
	int	l1, l2, l3;
	char	*key;
	
	if (!src) return data->jnum + data->jbnd * 2;
	
	if ((l1 = src->_vptr->conv(src, 'k', &key)) < 0) return l1;
	
	if (!l1) return data->jnum = data->jbnd = 0;
	
	switch (key[0]) {
		case 'F': data->jnum = 0; data->jbnd = 0; return l1;
		case 'f': data->jnum = 1; data->jbnd = 0; return l1;
		case 'B': data->jnum = 0; data->jbnd = 1; break;
		case 'b': data->jnum = 1; data->jbnd = 1; break;
		default:  errno = EINVAL; return -2;
	}
	l3 = 0;
	
	if ((l2 = src->_vptr->conv(src, 'i', data->mbnd)) <= 0) {
		l2 = 0; data->mbnd[1] = data->mbnd[0] = data->ivp.neqs;
	}
	else if ((l3 = src->_vptr->conv(src, 'i', data->mbnd+1)) <= 0 ) {
		l3 = 0; data->mbnd[1] = data->mbnd[0];
	}
	data->mbnd[2] = data->mbnd[0] + data->mbnd[1] + 1;
	data->mbnd[3] = 2*data->mbnd[0] + data->mbnd[1] + 1;
	
	return l1 + l2 + l3;
}
static int setH0(MPT_SOLVER_STRUCT(mebdfi) *data, MPT_INTERFACE(source) *src)
{
	int	len;
	
	if (!src) return data->h ? 1 : 0;
	if (!(len = src->_vptr->conv(src, 'd', &data->h))) data->h = 0.0;
	return len;
}
static int setMaxNSteps(MPT_SOLVER_STRUCT(mebdfi) *data, MPT_INTERFACE(source) *src)
{
	int	len, *addr = ((int *) data->iwork.iov_base) + 13;
	
	if (!src) return *addr;
	if (!(len = src->_vptr->conv(src, 'i', addr))) *addr = 0;
	return len;
}
static int setNind1(MPT_SOLVER_STRUCT(mebdfi) *data, MPT_INTERFACE(source) *src)
{
	int	len, *addr = (int *) data->iwork.iov_base;
	
	if (!src) return *addr;
	if (!(len = src->_vptr->conv(src, 'i', addr))) *addr = 0;
	return len;
}
static int setNind2(MPT_SOLVER_STRUCT(mebdfi) *data, MPT_INTERFACE(source) *src)
{
	int	len, *addr = ((int *) data->iwork.iov_base) + 1;
	
	if (!src) return *addr;
	if (!(len = src->_vptr->conv(src, 'i', addr))) *addr = 0;
	return len;
}
static int setNind3(MPT_SOLVER_STRUCT(mebdfi) *data, MPT_INTERFACE(source) *src)
{
	int	len, *addr = ((int *) data->iwork.iov_base) + 2;
	
	if (!src) return *addr;
	if (!(len = src->_vptr->conv(src, 'i', addr))) *addr = 0;
	return len;
}
static int setYP(MPT_SOLVER_STRUCT(mebdfi) *data, MPT_INTERFACE(source) *src)
{
	double	*yp;
	int	len;
	
	if (!src) return data->yp.iov_len / sizeof(double);
	
	if ((len = data->ivp.neqs * (data->ivp.pint + 1)) <= 0) {
		errno = EOVERFLOW; return -1;
	}
	if (!(yp = mpt_vecpar_alloc(&data->yp, len, sizeof(*yp))))
		return -1;
	
	while (src->_vptr->conv(src, 'd', yp) > 0) {
		yp++; if (!--len) return 0;
	}
	return (data->yp.iov_len -= len * sizeof(*yp)) / sizeof(*yp);
}

extern int mpt_mebdfi_property(MPT_SOLVER_STRUCT(mebdfi) *data, MPT_STRUCT(property) *prop, MPT_INTERFACE(source) *src)
{
	const char *name;
	intptr_t pos = -1, id;
	
	if (!prop) return (src && data) ? setIvp(data, src) : MPT_ENUM(TypeSolver);
	
	if (!(name = prop->name)) {
		if (src || ((pos = (intptr_t) prop->desc) < 0)) {
			errno = EINVAL;
			return -3;
		}
	}
	else if (!*name) {
		id = MPT_SOLVER_ENUM(ODE) | MPT_SOLVER_ENUM(DAE) | MPT_SOLVER_ENUM(PDE);
		if (data && src && (id = setIvp(data, src)) < 0) return id;
		prop->name = "mebdfi"; prop->desc = "implicit DAE solver with BDF";
		prop->val.fmt = "iid"; prop->val.ptr = &data->ivp;
		return id;
	}
	if (name && !strcasecmp(name, "version")) {
		static const char version[] = MPT_VERSION"\0";
		prop->name = "version"; prop->desc = "solver release information";
		prop->val.fmt= 0; prop->val.ptr = version;
		return 0;
	}
	
	id = 0;
	if (name ? !strcasecmp(name, "atol") : pos == id++) {
		if (data && (id = mpt_vecpar_value(&data->atol, &prop->val, src)) < 0) return id;
		prop->name = "atol"; prop->desc = "absolute tolerances";
		return id;
	}
	if (name ? !strcasecmp(name, "rtol") : pos == id++) {
		if (data && (id = mpt_vecpar_value(&data->rtol, &prop->val, src)) < 0) return id;
		prop->name = "rtol"; prop->desc = "relative tolerances";
		return id;
	}
	if (name ? !strncasecmp(name, "jac", 3) : pos == id++) {
		if (data && (id = setJacobian(data, src)) < 0) return id;
		prop->name = "jacobian"; prop->desc = "(user) jacobian parameters";
		prop->val.fmt = "ii"; prop->val.ptr = data->mbnd;
		return id;
	}
	if (name ? (!strcasecmp(name, "h0") || !strcasecmp(name, "stepinit")) : pos == id++) {
		if (data && (id = setH0(data, src)) < 0) return id;
		prop->name = "stepinit"; prop->desc = "explicit initial step size";
		prop->val.fmt = "d"; prop->val.ptr = &data->h;
		return id;
	}
	if (name ? (!strcasecmp(name, "maxstp") || !strcasecmp(name, "maxstep") || !strcasecmp(name, "iwork14")) : pos == id++) {
		if (data && (id = setMaxNSteps(data, src)) < 0) return id;
		prop->name = "maxstep"; prop->desc = "max. internal steps per call";
		prop->val.fmt = "i"; prop->val.ptr = (data && data->iwork.iov_base) ? ((int *) data->iwork.iov_base)+13 : 0;
		return id;
	}
	if (name ? !strcasecmp(name, "nind1") : pos == id++) {
		if (data && (id = setNind1(data, src)) < 0) return id;
		prop->name = "nind1"; prop->desc = "index1 variables";
		prop->val.fmt = "i"; prop->val.ptr = (data && data->iwork.iov_base) ? ((int *) data->iwork.iov_base) : 0;
		return id;
	}
	if (name ? !strcasecmp(name, "nind2") : pos == id++) {
		if (data && (id = setNind2(data, src)) < 0) return id;
		prop->name = "nind2"; prop->desc = "index2 variables";
		prop->val.fmt = "i"; prop->val.ptr = (data && data->iwork.iov_base) ? ((int *) data->iwork.iov_base)+1 : 0;
		return id;
	}
	if (name ? !strcasecmp(name, "nind3") : pos == id++) {
		if (data && (id = setNind3(data, src)) < 0) return id;
		prop->name = "nind3"; prop->desc = "index3 variables";
		prop->val.fmt = "i"; prop->val.ptr = (data && data->iwork.iov_base) ? ((int *) data->iwork.iov_base)+2 : 0;
		return id;
	}
	if (name ? !strncasecmp(name, "yp", 2) : pos == id++) {
		static const char fmt[] = { 'd' | (char) MPT_ENUM(TypeVector) };
		if (data && (id = setYP(data, src)) < 0) return id;
		prop->name = "yp"; prop->desc = "max. internal steps per call";
		prop->val.fmt = fmt; prop->val.ptr = data ? &data->yp : 0;
		return id;
	}
	
	errno = EINVAL;
	
	return -1;
}

