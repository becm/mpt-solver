/*!
 * set MINPACK solver parameter
 */

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>

#include "minpack.h"

#include "version.h"
#include "meta.h"

#include "module_functions.h"

static int setJacobian(MPT_SOLVER_STRUCT(minpack) *mp, const MPT_INTERFACE(metatype) *src)
{
	MPT_STRUCT(consumable) val;
	const char *key;
	int32_t ml, mu;
	int ret;
	
	if (!src) {
		mp->mu = mp->ml = 0;
		return 0;
	}
	key = 0;
	if ((ret = mpt_consumable_setup(&val, src)) < 0) {
		if ((ret = src->_vptr->conv(src, 'k', &key)) < 0) {
			return ret;
		}
		ret = 0;
	}
	else if ((ret = mpt_consume_key(&val, &key)) < 0) {
		return ret;
	} else {
		ret = 1;
	}
	if (!key || !*key) {
		mp->mu = mp->ml = 0;
		return ret;
	}
	switch (*key) {
		case 'F': mp->mu = mp->ml = 0; return ret;
		case 'f': mp->mu = mp->ml = 0; return ret;
		case 'B': case 'b': break;
		default:
			return MPT_ERROR(BadValue);
	}
	if ((ret = mpt_consume_int(&val, &ml)) < 0) {
		return ret;
	}
	if (!ret) {
		mp->mu = mp->ml = 0;
		return 1;
	}
	if (ml < 0 || ml >= mp->nls.nres) {
		return MPT_ERROR(BadValue);
	}
	if ((ret = mpt_consume_int(&val, &mu)) < 0) {
		return ret;
	}
	if (!ret) {
		mp->mu = mp->ml = ml;
		return 2;
	}
	if (mu < 0 || mu > mp->nls.nres) {
		return MPT_ERROR(BadValue);
	}
	mp->ml = ml;
	mp->mu = mu;
	return 3;
}
static int setDiag(MPT_SOLVER_STRUCT(minpack) *data, const MPT_INTERFACE(metatype) *src)
{
	MPT_INTERFACE(iterator) *it = 0;
	double *diag;
	int ret, nd, i;
	
	if (!src) {
		return data->diag.iov_len / sizeof(double);
	}
	if ((ret = src->_vptr->conv(src, MPT_type_pointer(MPT_ENUM(TypeIterator)), &it)) < 0) {
		return ret;
	}
	nd = data->nls.nval;
	if (!(diag = malloc(nd * sizeof(*diag)))) {
		return MPT_ERROR(BadOperation);
	}
	if ((ret = MPT_SOLVER_MODULE_FCN(data_set)(diag, nd, 0, it)) < 0) {
		free(diag);
		return ret;
	}
	for (i = ret; i < nd; ++i) diag[i] = 1.0;
	
	free(data->diag.iov_base);
	data->diag.iov_base = diag;
	data->diag.iov_len  = nd * sizeof(*diag);
	
	return ret;
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
extern int mpt_minpack_set(MPT_SOLVER_STRUCT(minpack) *mp, const char *name, const MPT_INTERFACE(metatype) *src)
{
	int ret;
	
	if (!name) {
		MPT_INTERFACE(iterator) *it = 0;
		double *par = mp->val.iov_base;
		long all = mp->nls.nval;
		
		if (all <= 0) {
			return MPT_ERROR(BadArgument);
		}
		if (src && (ret = src->_vptr->conv(src, MPT_type_pointer(MPT_ENUM(TypeIterator)), &it)) < 0) {
			return ret;
		}
		if (mp->nls.nres) {
			all += mp->nls.nres;
		} else {
			all *= 2;
		}
		if (!(par = mpt_solver_module_valloc(&mp->val, all, sizeof(*par)))) {
			return MPT_ERROR(BadOperation);
		}
		return MPT_SOLVER_MODULE_FCN(data_set)(par, mp->nls.nval, 0, ret ? it : 0);
	}
	if (!*name) {
		MPT_NLS_STRUCT(parameters) nls = MPT_NLSPAR_INIT;
		
		if (src && (ret =  mpt_solver_module_nlsset(&nls, src)) < 0) {
			return ret;
		}
		mpt_minpack_fini(mp);
		mpt_minpack_init(mp);
		mp->nls = nls;
		return ret;
	}
	if (!strcasecmp(name, "xtol")) {
		double val = 0;
		if (src && (ret = src->_vptr->conv(src, 'd', &val)) < 0) return ret;
		mp->xtol = val;
		return 0;
	}
	if (!strcasecmp(name, "ftol")) {
		double val = 1e-7;
		if (src && (ret = src->_vptr->conv(src, 'd', &val)) < 0) return ret;
		mp->ftol = val;
		return 0;
	}
	if (!strcasecmp(name, "gtol")) {
		double val = 0;
		if (src && (ret = src->_vptr->conv(src, 'd', &val)) < 0) return ret;
		mp->gtol = val;
		return 0;
	}
	if (!strncasecmp(name, "jac", 3)) {
		return setJacobian(mp, src);
	}
	if (!strcasecmp(name, "maxfev")) {
		uint32_t val = 0;
		if (src && (ret = src->_vptr->conv(src, 'u', &val)) < 0) return ret;
		mp->maxfev = val;
		return 0;
	}
	if (!strncasecmp(name, "fac", 3)) {
		double val = 200;
		if (src && (ret = src->_vptr->conv(src, 'd', &val)) < 0) return ret;
		mp->factor = val;
		return 0;
	}
	if (!strncasecmp(name, "eps", 3)) {
		double val = 0;
		if (src && (ret = src->_vptr->conv(src, 'd', &val)) < 0) return ret;
		mp->epsfcn = val;
		return 0;
	}
	if (!strncasecmp(name, "nprint", 3)) {
		char val = 0;
		if (src && (ret = src->_vptr->conv(src, 'c', &val)) < 0) return ret;
		mp->nprint = val;
		return 0;
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
		return MPT_SOLVER_ENUM(NlsUser) | MPT_SOLVER_ENUM(NlsJac);
	}
	if (!(name = prop->name)) {
		if ((pos = (intptr_t) prop->desc) < 0) {
			return MPT_ERROR(BadArgument);
		}
	}
	else if (!*name) {
		prop->name = "minpack";
		prop->desc = "solver for (overdetermined) nonlinear equotations";
		return mpt_solver_module_value_nls(&prop->val, mp ? &mp->nls : 0);
	}
	if (name && !strcasecmp(name, "version")) {
		static const char version[] = BUILD_VERSION"\0";
		prop->name = "version";
		prop->desc = "solver release information";
		prop->val.fmt = 0;
		prop->val.ptr = version;
		return 0;
	}
	
	id = -1;
	if (name ? !strcasecmp(name, "xtol") : (pos == ++id)) {
		prop->name = "xtol";
		prop->desc = "desired relative error";
		mpt_solver_module_value_double(&prop->val, &mp->xtol);
		if (!mp) return id;
		return mp->xtol ? 1 : 0;
	}
	if (name ? !strcasecmp(name, "ftol") : (pos == ++id)) {
		prop->name = "ftol";
		prop->desc = "desired residual";
		mpt_solver_module_value_double(&prop->val, &mp->ftol);
		if (!mp) return id;
		return (mp->ftol != 1e-7) ? 1 : 0;
	}
	if (name ? !strcasecmp(name, "gtol") : (pos == ++id)) {
		prop->name = "gtol";
		prop->desc = "desired orthogonality";
		mpt_solver_module_value_double(&prop->val, &mp->gtol);
		if (!mp) return id;
		return mp->gtol ? 1 : 0;
	}
	if (name ? !strncasecmp(name, "jac", 3) : (pos == ++id)) {
		prop->name = "jacobian";
		prop->desc = "(user) jacobian settings";
		mpt_solver_module_value_int(&prop->val, &mp->mu);
		if (!mp) return id;
		return (mp->mu || mp->ml) ? 2 : 0;
	}
	if (name ? !strcasecmp(name, "maxfev") : (pos == ++id)) {
		prop->name = "maxfev";
		prop->desc = "max. function evaluations per call";
		mpt_solver_module_value_int(&prop->val, &mp->maxfev);
		if(!mp) return id;
		return mp->maxfev ? 1 : 0;
	}
	if (name ? !strncasecmp(name, "fac", 3) : (pos == ++id)) {
		prop->name = "factor";
		prop->desc = "initial step bound";
		mpt_solver_module_value_double(&prop->val, &mp->factor);
		if (!mp) return id;
		return (mp->factor == 200 * (mp->nls.nval + 1)) ? 0 : 1;
	}
	if (name ? !strncasecmp(name, "eps", 3) : (pos == ++id)) {
		prop->name = "epsfcn";
		prop->desc = "initial forward-difference step length";
		mpt_solver_module_value_double(&prop->val, &mp->epsfcn);
		if (!mp) return id;
		return mp->epsfcn ? 1 : 0;
	}
	if (name ? !strncasecmp(name, "nprint", 3) : (pos == ++id)) {
		static const uint8_t fmt[] = "c";
		prop->name = "nprint";
		prop->desc = "iteration output";
		prop->val.fmt = fmt;
		prop->val.ptr = &mp->nprint;
		if (!mp) return id;
		return mp->nprint ? 1 : 0;
	}
	if (name ? !strcasecmp(name, "diag") : (pos == ++id)) {
		static const uint8_t fmt[] = { MPT_type_vector('d'), 0 };
		prop->name = "diag";
		prop->desc = "scale factor for variables";
		prop->val.fmt = fmt;
		prop->val.ptr = &mp->diag;
		if (!mp) return id;
		return mp->diag.iov_len / sizeof(double);
	}
	return MPT_ERROR(BadArgument);
}
