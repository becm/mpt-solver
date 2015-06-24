/*!
 * set vode integrator parameter.
 */

#include <string.h>
#include <strings.h>
#include <limits.h>
#include <float.h>
#include <errno.h>
#include <ctype.h>

#include "release.h"
#include "version.h"

#include "vode.h"

static int setIvp(MPT_SOLVER_STRUCT(vode) *data, MPT_INTERFACE(source) *src)
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
		mpt_vode_fini(data);
		if (mpt_vode_init(data) < 0) {
			return -1;
		}
		data->ivp = ivp;
		
		((double *) data->rwork.iov_base)[12] = ivp.last;
		
		return ret;
	}
}
static int setJacobian(MPT_SOLVER_STRUCT(vode) *data, MPT_INTERFACE(source) *src)
{
	char *key;
	int l1, l2, l3, *iwk;
	
	if (!src) return data->miter;
	
	if ((l1 = src->_vptr->conv(src, 'k', &key)) < 0) return -2;
	
	if (!l1) return data->miter = 0;
	
	l2 = l3 = 0;
	iwk = data->iwork.iov_base;
	
	switch (key[0]) {
		case 'n': case 'N': data->miter = 0; return l1;
		case 'F': data->miter = 1; return l1;
		case 'f': data->miter = 2; return l1;
		case 'd': case 'D': data->miter = 3; return l1;
		case 'B': case 'b': break;
		default:  errno = EINVAL; return -1;
	}
	if ((l2 = src->_vptr->conv(src, 'i', iwk)) <= 0) {
		l2 = 0; iwk[1] = iwk[0] = data->ivp.neqs;
	}
	else if ((l3 = src->_vptr->conv(src, 'i', iwk+1)) <= 0) {
		l3 = 0; iwk[1] = iwk[0];
	}
	data->miter = (key[0] == 'B') ? 4 : 5;
	
	return l1 + l2 + l3;
}
static int setStepType(MPT_SOLVER_STRUCT(vode) *data, MPT_INTERFACE(source) *src)
{
	char	*key;
	int	l1, l2;
	
	if (!src) return data->itask;
	
	if ((l1 = src->_vptr->conv(src, 'k', &key)) < 0) return -2;
	
	if (!l1) { data->itask = 1; return 0; }
	
	switch (key[0]) {
		/* single step */
		case 's': case '2': data->itask = 2; return l1;
		/* disable interpolation */
		case 'n': case 'N': case '3': data->itask = 3; return l1;
		/* enable interpolation */
		case 'i': case '1': data->itask = 1; return l1;
		/* opteration till tstop */
		case 'I': case 'S': case '4': case '5':
			if ((l2 = src->_vptr->conv(src, 'd', data->rwork.iov_base)) > 0)
				break;
		default:  errno = EINVAL; return -1;
	}
	data->itask = (key[0] == 'S' || key[0] == '5') ? 5 : 4;
	
	return l1 + l2;
}
static int setMethod(MPT_SOLVER_STRUCT(vode) *data, MPT_INTERFACE(source) *src)
{
	int	len;
	char	*key;
	
	if (!src) return data->meth;
	
	if ((len = src->_vptr->conv(src, 'k', &key)) < 0) return -3;
	
	if (!len) { data->meth = 1; return 0; }
	
	switch ( key[0] ) {
		case 'a': data->jsv = -1; data->meth = 1; return len;
		case 'A': data->jsv =  1; data->meth = 1; return len;
		case 'b': data->jsv = -1; data->meth = 2; return len;
		case 'B': data->jsv =  1; data->meth = 2; return len;
		default: return -1;
	}
}
static int setMaxOrd(MPT_SOLVER_STRUCT(vode) *data, MPT_INTERFACE(source) *src)
{
	int	len, *iwk = data->iwork.iov_base;
	
	if (!src) return data->iopt ? iwk[4] : 0;
	
	if ((len = src->_vptr->conv(src, 'i', iwk+4)) < 0) return len;
	
	if (!len) return iwk[4] = 0;
	data->iopt = 6;
	return len;
}
static int setMaxNSteps(MPT_SOLVER_STRUCT(vode) *data, MPT_INTERFACE(source) *src)
{
	int	len, *iwk = data->iwork.iov_base;
	
	if (!src) return data->iopt ? iwk[5] : 0;
	
	if ((len = src->_vptr->conv(src, 'i', iwk+5)) < 0) return len;
	
	if (!len) return iwk[5] = 0;
	data->iopt = 7;
	return len;
}
static int setMaxHNil(MPT_SOLVER_STRUCT(vode) *data, MPT_INTERFACE(source) *src)
{
	int	len, *iwk = data->iwork.iov_base;
	
	if (!src) return data->iopt ? iwk[6] : 0;
	
	if ((len = src->_vptr->conv(src, 'i', iwk+6)) < 0) return len;
	
	if (!len) return iwk[6] = 0;
	data->iopt = 8;
	return len;
}
static int setInitStep(MPT_SOLVER_STRUCT(vode) *data, MPT_INTERFACE(source) *src)
{
	double	*rwk = data->rwork.iov_base;
	int	len;
	
	if (!src) return (data->iopt && rwk[4]) ? 1 : 0;
	
	if ((len = src->_vptr->conv(src, 'd', rwk+4)) < 0) return len;
	
	if (!len) return rwk[4] = 0;
	data->iopt = 9;
	return len;
}
static int setMaxStep(MPT_SOLVER_STRUCT(vode) *data, MPT_INTERFACE(source) *src)
{
	double	*rwk = data->rwork.iov_base;
	int	len;
	
	if (!src) return (data->iopt && rwk[5]) ? 1 : 0;
	
	if ((len = src->_vptr->conv(src, 'd', rwk+5)) < 0) return len;
	
	if (!len) return rwk[5] = 0;
	data->iopt = 10;
	return len;
}
static int setMinStep(MPT_SOLVER_STRUCT(vode) *data, MPT_INTERFACE(source) *src)
{
	double	*rwk = data->rwork.iov_base;
	int	len;
	
	if (!src) return (data->iopt && rwk[6]) ? 1 : 0;
	
	if ((len = src->_vptr->conv(src, 'd', rwk+6)) < 0) return len;
	
	if (!len) return rwk[6] = 0;
	data->iopt = 11;
	return len;
}

extern int mpt_vode_property(MPT_SOLVER_STRUCT(vode) *data, MPT_STRUCT(property) *prop, MPT_INTERFACE(source) *src)
{
	const char *name;
	intptr_t pos = 0, id;
	
	if (!prop) {
		return (src && data) ? setIvp(data, src) : MPT_ENUM(TypeSolver);
	}
	if (!(name = prop->name)) {
		if (src || !((pos = (intptr_t) prop->desc) < 0)) {
			errno = EINVAL;
			return -3;
		}
	}
	else if (!*name) {
		id = MPT_SOLVER_ENUM(ODE) | MPT_SOLVER_ENUM(PDE);
		if (data && src && (id = setIvp(data, src)) < 0) return id;
		prop->name = "vode"; prop->desc = "implicit DAE solver with BDF";
		prop->val.fmt  = "iid"; prop->val.ptr = &data->ivp;
		return id;
	}
	id = 0;
	
	if (name && !strcasecmp(name, "version")) {
		static const char version[] = MPT_VERSION"\0";
		prop->name = "version"; prop->desc = "solver release information";
		prop->val.fmt = 0; prop->val.ptr = version;
		return 0;
	}
	
	if (name ? !strcasecmp(name, "atol") : (pos == id++)) {
		if (data && (id = mpt_vecpar_value(&data->atol, &prop->val, src)) < 0) return id;
		prop->name = "atol"; prop->desc = "absolute tolerances";
		return id;
	}
	if (name ? !strcasecmp(name, "rtol") : (pos == id++)) {
		if (data && (id = mpt_vecpar_value(&data->rtol, &prop->val, src)) < 0) return id;
		prop->name = "rtol"; prop->desc = "relative tolerances";
		return id;
	}
	if (name ? !strncasecmp(name, "jac", 3) : (pos == id++)) {
		if (data && (id = setJacobian(data, src)) < 0) return id;
		prop->name = "jacobian"; prop->desc = "(user) jacobian parameters";
		prop->val.fmt  = "b"; prop->val.ptr = &data->miter;
		return id;
	}
	if (name ? !strncasecmp(name, "itask", 2) : (pos == id++)) {
		if (data && (id = setStepType(data, src)) < 0) return id;
		prop->name = "itask"; prop->desc = "step control";
		prop->val.fmt  = "h"; prop->val.ptr = &data->itask;
		return id;
	}
	if (name ? !strncasecmp(name, "method", 4) : (pos == id++)) {
		if (data && (id = setMethod(data, src)) < 0) return id;
		prop->name = "method"; prop->desc = "iteration method";
		prop->val.fmt  = "b"; prop->val.ptr = &data->meth;
		return id;
	}
	/* integer array parameter */
	if (name ? (!strcasecmp(name, "maxord") || !strcasecmp(name, "iwork5")) : (pos == id++)) {
		if (data && (id = setMaxOrd(data, src)) < 0) return id;
		prop->name = "maxord"; prop->desc = "maximum order to be allowed";
		prop->val.fmt  = "i"; prop->val.ptr = (data && data->iwork.iov_base) ? ((int *) data->iwork.iov_base) + 4 : 0;
		return id;
	}
	if (name ? (!strcasecmp(name, "mxstep") || !strcasecmp(name, "iwork6")) : (pos == id++)) {
		if (data && (id = setMaxNSteps(data, src)) < 0) return id;
		prop->name = "mxstep"; prop->desc = "max. internal steps per call";
		prop->val.fmt  = "i"; prop->val.ptr = (data && data->iwork.iov_base) ? ((int *) data->iwork.iov_base) + 5 : 0;
		return id;
	}
	if (name ? (!strcasecmp(name, "mxhnil") || !strcasecmp(name, "iwork7")) : (pos == id++)) {
		if (data && (id = setMaxHNil(data, src)) < 0) return id;
		prop->name = "mxhnil"; prop->desc = "max. warnings for 't + h = t'";
		prop->val.fmt  = "i"; prop->val.ptr = (data && data->iwork.iov_base) ? ((int *) data->iwork.iov_base) + 6 : 0;
		return id;
	}
	/* real array parameter */
	if (name ? (!strcasecmp(name, "h0") || !strcasecmp(name, "rwork5")) : (pos == id++)) {
		if (data && (id = setInitStep(data, src)) < 0) return id;
		prop->name = "h0"; prop->desc = "explicit initial steps size";
		prop->val.fmt  = "d"; prop->val.ptr = (data && data->rwork.iov_base) ? ((double *) data->rwork.iov_base) + 4 : 0;
		return id;
	}
	if (name ? (!strcasecmp(name, "hmax") || !strcasecmp(name, "rwork6")) : (pos == id++)) {
		if (data && (id = setMaxStep(data, src)) < 0) return id;
		prop->name = "hmax"; prop->desc = "maximal internal steps size";
		prop->val.fmt = "d"; prop->val.ptr = (data && data->rwork.iov_base) ? ((double *) data->rwork.iov_base) + 5 : 0;
		return setMaxStep(data, src);
	}
	if (name ? (!strcasecmp(name, "hmin") || !strcasecmp(name, "rwork7")) : (pos == id++)) {
		if (data && (id = setMinStep(data, src)) < 0) return id;
		prop->name = "hmin"; prop->desc = "maximal internal steps size";
		prop->val.fmt  = "d"; prop->val.ptr = (data && data->rwork.iov_base) ? ((double *) data->rwork.iov_base) + 5 : 0;
		return id;
	}
	
	errno = EINVAL;
	
	return -1;
}
