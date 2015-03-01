/*!
 * set parameter for limex integrator descriptor.
 */

#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <errno.h>

#include "release.h"
#include "version.h"

#include "limex.h"

static int setIvp(MPT_SOLVER_STRUCT(limex) *data, MPT_INTERFACE(source) *src)
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
		mpt_limex_reset(data);
		data->ivp = ivp;
		return ret;
	}
}
static int setJacobian(MPT_SOLVER_STRUCT(limex) *data, MPT_INTERFACE(source) *src)
{
	int	l1, l2, l3;
	char	*key = "F";
	
	if (!src) return data->iopt[6];
	
	if ((l1 = src->_vptr->conv(src, 'k', &key)) < 0) {
		return -2;
	}
	if (!l1) return data->iopt[6] = 0;
	
	switch ( key[0] ) {
		case 'F':
			data->iopt[6] = 1;
		case 'f':
			l2 = data->ivp.neqs * (data->ivp.pint + 1);
			data->iopt[7] = data->iopt[8] = l2;
			return l1;
		case 'B':
			data->iopt[6] = 1;
		case 'b':
			l3 = 0;
			if ((l2 = src->_vptr->conv(src, 'i', data->iopt+7)) <= 0) {
				l2 = 0; data->iopt[8] = data->iopt[7] = data->ivp.neqs;
			}
			else if ((l3 = src->_vptr->conv(src, 'i', data->iopt+8)) <= 0) {
				l3 = 0; data->iopt[8] = data->iopt[7];
			}
			return l1 + l2 + l3;
		default:  errno = EINVAL; return -3;
	}
}
static int setStepSize(MPT_SOLVER_STRUCT(limex) *data, MPT_INTERFACE(source) *src)
{
	int len;
	
	if (!src) return data->h ? 1 : 0;
	if ((len = src->_vptr->conv(src, 'd', &data->h)) == 0) data->h = 0.0;
	return len;
}
static int setYS(MPT_SOLVER_STRUCT(limex) *data, MPT_INTERFACE(source) *src)
{
	int total = 0, curr, ny;
	double *ys;
	
	if (!src) return data->ys.iov_len / sizeof(double);
	
	if ((ny = data->ivp.neqs * (data->ivp.pint + 1)) < 0) {
		errno = EOVERFLOW;
		return -1;
	}
	if (!(ys = mpt_vecpar_alloc(&data->ys, ny, sizeof(*ys)))) {
		return -1;
	}
	while ((curr = src->_vptr->conv(src, 'd', ys)) > 0) {
		total += curr; ys++;
		if (!--ny) {
			return total;
		}
	}
	data->ys.iov_len -= ny * sizeof(double);
	
	return total;
}

extern int mpt_limex_property(MPT_SOLVER_STRUCT(limex) *data, MPT_STRUCT(property) *prop, MPT_INTERFACE(source) *src)
{
	const char *name;
	intptr_t pos = -1, id;
	
	if (!prop) return (data && src) ? setIvp(data, src) : MPT_ENUM(TypeSolver);
	
	if (!(name = prop->name)) {
		if (src || ((pos = (intptr_t) prop->desc) < 0)) {
			errno = EINVAL;
			return -3;
		}
	}
	else if (!*name) {
		id = MPT_SOLVER_ENUM(ODE) | MPT_SOLVER_ENUM(DAE) | MPT_SOLVER_ENUM(PDE);
		if (data && src && (id = setIvp(data, src)) < 0) return id;
		prop->name = "limex"; prop->desc = "extrapolation integrator for linearly-implicit DAE";
		prop->fmt  = "iid"; prop->data = &data->ivp;
		return id;
	}
	if (name && !strcasecmp(name, "version")) {
		static const char version[] = MPT_VERSION"\0";
		prop->name = "version"; prop->desc = "solver release information";
		prop->fmt = 0; prop->data = version;
		return 0;
	}
	
	id = 0;
	if (name ? !strcasecmp(name, "atol") : pos == id++) {
		if (data && (id = mpt_vecpar_property(&data->atol, prop, src)) < 0) return id;
		prop->name = "atol"; prop->desc = "absolute tolerances";
		return id;
	}
	if ( name ? !strcasecmp(name, "rtol") : pos == id++) {
		if (data && (id = mpt_vecpar_property(&data->rtol, prop, src)) < 0) return id;
		prop->name = "rtol"; prop->desc = "relative tolerances";
		return id;
	}
	if ( name ? !strncasecmp(name, "jac", 3) : pos == id++) {
		if (data && (id = setJacobian(data, src)) < 0) return id;
		prop->name = "jacobian"; prop->desc = "(user) jacobian settings";
		prop->fmt  = "iii"; prop->data = data->iopt + 6;
		return id;
	}
	if ( name ? (!strcasecmp(name, "h") || !strcasecmp(name, "initstep")) : pos == id++) {
		if (data && (id = setStepSize(data, src)) < 0) return id;
		prop->name = "h"; prop->desc = "initial/next stepsize";
		prop->fmt  = "d"; prop->data = &data->h;
		return id;
	}
	/* integer parameter */
	if (name ? (!strcasecmp(name, "monitor") || !strcasecmp(name, "iopt1")) : pos == id++) {
		if (src) return -1;
		prop->name = "monitor"; prop->desc = "integration monitor output";
		prop->fmt  = "i"; prop->data = data->iopt;
		return id;
	}
	if (name ? (!strcasecmp(name, "solout") || !strcasecmp(name, "iopt3")) : pos == id++) {
		if (src) return -1;
		prop->name = "solout"; prop->desc = "(intermediate) solution output";
		prop->fmt  = "i"; prop->data = data->iopt+2;
		return id;
	}
	if (name ? (!strncasecmp(name, "bnos", 4) || !strcasecmp(name, "iopt5")) : pos == id++) {
		if (src) return -1;
		prop->name = "bnosingular"; prop->desc = "B-matrix may not be singular";
		prop->fmt  = "i"; prop->data = data->iopt+4;
		return id;
	}
	if (name ? (!strcasecmp(name, "jacreuse") || !strcasecmp(name, "iopt10")) : pos == id++) {
		if (src) return -1;
		prop->name = "jacreuse"; prop->desc = "try to reuse jacobian";
		prop->fmt  = "i"; prop->data = data->iopt+9;
		return id;
	}
	if (name ? (!strcasecmp(name, "single") || !strcasecmp(name, "iopt12")) : pos == id++) {
		if (src) return -1;
		prop->name = "single"; prop->desc = "enable single step modes";
		prop->fmt  = "i"; prop->data = data->iopt+11;
		return id;
	}
	if (name ? !strcasecmp(name, "denseout") : pos == id++) {
		if (src) return -1;
		prop->name = "denseout"; prop->desc = "dense output settings";
		prop->fmt  = "ii"; prop->data = data->iopt+12;
		return id;
	}
	if (name ? (!strcasecmp(name, "tend") || !strcasecmp(name, "ropt3")) : pos == id++) {
		if (src) return -1;
		prop->name = "tend"; prop->desc = "dense output settings";
		prop->fmt  = "d"; prop->data = data->ropt+2;
		return id;
	}
	if (name ? (!strcasecmp(name, "plotjac") || !strcasecmp(name, "iopt18")) : pos == id++) {
		if (src) return -1;
		prop->name = "plotjac"; prop->desc = "dense output settings";
		prop->fmt  = "i"; prop->data = data->iopt+17;
		return id;
	}
	if (name ? (!strcasecmp(name, "maxstep") || !strcasecmp(name, "ropt1")) : pos == id++) {
		if (src) return -1;
		prop->name = "ipos"; prop->desc = "maximum internal step size";
		prop->fmt  = "d"; prop->data = data->ropt;
		return id;
	}
	if (name ? !strcasecmp(name, "ipos") : pos == id++) {
		if (src) return -1;
		prop->name = "ipos"; prop->desc = "maximum internal step size";
		prop->fmt  = "i"; prop->data = data->ipos.iov_base;
		return id;
	}
	if (name ? !strncasecmp(name, "yprime", 2) : pos == id++) {
		static const char fmt[] = { 'd' | (char) MPT_ENUM(TypeVector) };
		if (data && (id = setYS(data, src)) < 0) return id;
		prop->name = "yprime"; prop->desc = "deviation at initial/current point";
		prop->fmt  = fmt; prop->data = &data->ipos;
		return id;
	}
	
	errno = EINVAL;
	return -3;
}
