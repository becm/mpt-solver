/*!
 * set MINPACK solver parameter
 */

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>

#include "version.h"

#include "minpack.h"

static int setJacobian(MPT_SOLVER_STRUCT(minpack) *mp, MPT_INTERFACE(metatype) *src)
{
	char *key;
	int32_t val;
	int res;
	
	if (!src) {
		mp->mu = mp->ml = 0;
		return 0;
	}
	if ((res = src->_vptr->conv(src, 'k' | MPT_ENUM(ValueConsume), &key)) < 0) {
		return res;
	}
	if (!res || !key) {
		return mp->mu = mp->ml = 0;
	}
	switch (key[0]) {
		case 'F': mp->mu = mp->ml = 0; return 1;
		case 'f': mp->mu = mp->ml = 0; return 1;
		case 'B': case 'b': break;
		default:
			return MPT_ERROR(BadValue);
	}
	if ((res = src->_vptr->conv(src, 'i' | MPT_ENUM(ValueConsume), &val)) < 0) {
		return res;
	}
	if (!res) {
		mp->mu = mp->ml = 0;
		return 1;
	}
	if ((res = src->_vptr->conv(src, 'i' | MPT_ENUM(ValueConsume), &mp->mu)) < 0) {
		return res;
	}
	if (!res) {
		mp->mu = mp->ml = val;
		return 2;
	}
	return 3;
}
static int setDiag(MPT_SOLVER_STRUCT(minpack) *data, MPT_INTERFACE(metatype) *src)
{
	double *diag;
	int total = 0, curr, nd;
	
	if (!src) return data->diag.iov_len / sizeof(double);
	
	nd = data->nls.nval;
	if (!(diag = malloc(nd * sizeof(*diag)))) {
		return MPT_ERROR(BadOperation);
	}
	if (src) {
		while ((curr = src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), diag))) {
			if (curr < 0) {
				free(diag);
				return curr;
			}
			if (++total >= nd) {
				break;
			}
			++diag;
		}
	}
	for (curr = total; curr < nd; ++curr) diag[curr] = 1.0;
	
	free(data->diag.iov_base);
	data->diag.iov_base = diag;
	data->diag.iov_len  = nd * sizeof(*diag);
	
	return total;
}

/*!
 * \ingroup mptNlSolvPortN2
 * \brief set N2 property
 * 
 * Query property of Port N2 solver
 * 
 * \param mp   N2 data
 * \param name name of property to change
 * \param src  data source to change property
 * 
 * \retval <0   failure
 * \retval >=0  used values
 */
extern int mpt_minpack_set(MPT_SOLVER_STRUCT(minpack) *mp, const char *name, MPT_INTERFACE(metatype) *src)
{
	int ret = 0;
	
	if (!name) {
		double *x = mp->val.iov_base;
		
		if (mp->nls.nval <= 0) {
			return MPT_ERROR(BadArgument);
		}
		if ((ret = mpt_vecpar_set(&x, mp->nls.nval, src)) < 0) {
			return ret;
		}
		if (!mp->val.iov_base) {
			mp->val.iov_base = x;
			mp->val.iov_len = mp->nls.nval * sizeof(*x);
		}
		return ret;
	}
	if (!*name) {
		MPT_SOLVER_NLS_STRUCT(parameters) nls = mp->nls;
		
		if (src && (ret =  mpt_nlspar_set(&nls, src)) < 0) {
			return ret;
		}
		mpt_minpack_fini(mp);
		mpt_minpack_init(mp);
		mp->nls = nls;
		return ret;
	}
	if (!strcasecmp(name, "xtol")) {
		double val = 0;
		if (src && (ret = src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), &val)) < 0) return ret;
		mp->xtol = val;
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "ftol")) {
		double val = 1e-7;
		if (src && (ret = src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), &val)) < 0) return ret;
		mp->ftol = val;
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "gtol")) {
		double val = 0;
		if (src && (ret = src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), &val)) < 0) return ret;
		mp->gtol = val;
		return ret ? 1 : 0;
	}
	if (!strncasecmp(name, "jac", 3)) {
		return setJacobian(mp, src);
	}
	if (!strcasecmp(name, "maxfev")) {
		uint32_t val = 0;
		if (src && (ret = src->_vptr->conv(src, 'u' | MPT_ENUM(ValueConsume), &val)) < 0) return ret;
		mp->maxfev = val;
		return ret ? 1 : 0;
	}
	if (!strncasecmp(name, "fac", 3)) {
		double val = 200;
		if (src && (ret = src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), &val)) < 0) return ret;
		mp->factor = val;
		return ret ? 1 : 0;
	}
	if (!strncasecmp(name, "eps", 3)) {
		double val = 0;
		if (src && (ret = src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), &val)) < 0) return ret;
		mp->epsfcn = val;
		return ret ? 1 : 0;
	}
	if (!strncasecmp(name, "nprint", 3)) {
		char val = 0;
		if (src && (ret = src->_vptr->conv(src, 'c' | MPT_ENUM(ValueConsume), &val)) < 0) return ret;
		mp->nprint = val;
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "diag")) {
		return setDiag(mp, src);
	}
	return MPT_ERROR(BadArgument);
}

/*!
 * \ingroup mptNlSolvMinpack
 * \brief set Minpack property
 * 
 * Query property of Minpack solver
 * 
 * \param mp   Minpack data
 * \param prop solver property
 * 
 * \retval 0   default value
 * \retval <0  failure
 * \retval >0  changed property
 */
extern int mpt_minpack_get(const MPT_SOLVER_STRUCT(minpack) *mp, MPT_STRUCT(property) *prop)
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
		prop->name = "minpack"; prop->desc = "solver for (overdetermined) nonlinear equotations";
		prop->val.fmt = "ii"; prop->val.ptr = &mp->nls;
		return MPT_SOLVER_ENUM(NlsVector) | MPT_SOLVER_ENUM(NlsOverDet);
	}
	if (name && !strcasecmp(name, "version")) {
		static const char version[] = BUILD_VERSION"\0";
		prop->name = "version"; prop->desc = "solver release information";
		prop->val.fmt = 0; prop->val.ptr = version;
		return 0;
	}
	
	id = -1;
	if (name ? !strcasecmp(name, "xtol") : (pos == ++id)) {
		prop->name = "xtol"; prop->desc = "desired relative error";
		prop->val.fmt = "d"; prop->val.ptr = &mp->xtol;
		if (!mp) return id;
		return mp->xtol ? 1 : 0;
	}
	if (name ? !strcasecmp(name, "ftol") : (pos == ++id)) {
		prop->name = "ftol"; prop->desc = "desired residual";
		prop->val.fmt = "d"; prop->val.ptr = &mp->ftol;
		if (!mp) return id;
		return (mp->ftol != 1e-7) ? 1 : 0;
	}
	if (name ? !strcasecmp(name, "gtol") : (pos == ++id)) {
		prop->name = "gtol"; prop->desc = "desired orthogonality";
		prop->val.fmt = "d"; prop->val.ptr = &mp->gtol; prop->val.fmt= "d";
		if (!mp) return id;
		return mp->gtol ? 1 : 0;
	}
	if (name ? !strncasecmp(name, "jac", 3) : (pos == ++id)) {
		prop->name = "jacobian"; prop->desc = "(user) jacobian settings";
		prop->val.fmt = "ii"; prop->val.ptr = &mp->mu;
		if (!mp) return id;
		return (mp->mu || mp->ml) ? 2 : 0;
	}
	if (name ? !strcasecmp(name, "maxfev") : (pos == ++id)) {
		prop->name = "maxfev"; prop->desc = "max. function evaluations per call";
		prop->val.fmt = "i"; prop->val.ptr = &mp->maxfev;
		if(!mp) return id;
		return mp->maxfev ? 1 : 0;
	}
	if (name ? !strncasecmp(name, "fac", 3) : (pos == ++id)) {
		prop->name = "factor"; prop->desc = "initial step bound";
		prop->val.fmt = "d"; prop->val.ptr = &mp->factor;
		if (!mp) return id;
		return (mp->factor == 200*(mp->nls.nval + 1)) ? 0 : 1;
	}
	if (name ? !strncasecmp(name, "eps", 3) : (pos == ++id)) {
		prop->name = "epsfcn"; prop->desc = "initial forward-difference step length";
		prop->val.fmt = "d"; prop->val.ptr = &mp->epsfcn;
		if (!mp) return id;
		return mp->epsfcn ? 1 : 0;
	}
	if (name ? !strncasecmp(name, "nprint", 3) : (pos == ++id)) {
		prop->name = "nprint"; prop->desc = "iteration output";
		prop->val.fmt = "c"; prop->val.ptr = &mp->nprint;
		if (!mp) return id;
		return mp->nprint ? 1 : 0;
	}
	if (name ? !strcasecmp(name, "diag") : (pos == ++id)) {
		static const char fmt[] = { MPT_value_toVector('d'), 0 };
		prop->name = "diag"; prop->desc = "scale factor for variables";
		prop->val.fmt = fmt; prop->val.ptr = &mp->diag;
		if (!mp) return id;
		return mp->diag.iov_len / sizeof(double);
	}
	return MPT_ERROR(BadArgument);
}
