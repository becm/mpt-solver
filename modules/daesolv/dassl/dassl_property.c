/*!
 * set dDASSL integrator parameters
 */

#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "dassl.h"

#include "version.h"
#include "types.h"
#include "meta.h"

#include "module_functions.h"

static int setInt(MPT_SOLVER_STRUCT(dassl) *da, size_t pos, int val)
{
	int *iwk = da->iwork.iov_base;
	size_t size = da->iwork.iov_len / sizeof(int);
	if (pos >= size) {
		if (!val) {
			return 0;
		}
		if (!(iwk = mpt_solver_module_valloc(&da->iwork, pos + 1, sizeof(int)))) {
			return MPT_ERROR(BadOperation);
		}
	}
	iwk[pos] = val;
	return 1;
}
static int setReal(MPT_SOLVER_STRUCT(dassl) *da, size_t pos, double val)
{
	double *rwk = da->rwork.iov_base;
	size_t size = da->rwork.iov_len / sizeof(double);
	if (pos >= size) {
		if (!val) {
			return 0;
		}
		if (!(rwk = mpt_solver_module_valloc(&da->rwork, pos + 1, sizeof(double)))) {
			return MPT_ERROR(BadOperation);
		}
	}
	rwk[pos] = val;
	return 1;
}

static int setJacobian(MPT_SOLVER_STRUCT(dassl) *da, MPT_INTERFACE(convertable) *src)
{
	MPT_INTERFACE(iterator) *it;
	const char *key;
	int32_t ld, ud;
	int ret, ujac;
	long max;
	char mode;
	
	if (!src) {
		da->info[4] = 1;
		da->info[5] = 0;
		return 0;
	}
	key = 0;
	it = 0;
	if ((ret = src->_vptr->convert(src, MPT_ENUM(TypeIteratorPtr), &it)) >= 0) {
		ret = mpt_solver_module_consume_value(it, 'k', &key, 0);
	}
	if (ret < 0 && (ret = src->_vptr->convert(src, 'k', &key)) < 0) {
		return ret;
	}
	if (!key || !(mode = *key)) {
		da->info[4] = 1;
		da->info[5] = 0;
		return ret;
	}
	switch (mode) {
		case 'F': da->info[4] = 1; da->info[5] = 0; return ret;
		case 'f': da->info[4] = 0; da->info[5] = 0; return ret;
		case 'B': ujac = 1; break;
		case 'b': ujac = 0; break;
		default:
			return MPT_ERROR(BadValue);
	}
	
	if (!it || (ret = mpt_solver_module_consume_value(it, 'i', &ld, sizeof(ld))) < 0) {
		ld = da->ivp.neqs;
		if (!da->ivp.pint) {
			--ld;
		}
		ud = ld;
	}
	else if (!ret) {
		ld = ud = da->ivp.neqs;
		ret = 2;
	}
	else if ((ret = mpt_solver_module_consume_value(it, 'i', &ud, sizeof(ud))) < 0) {
		return ret;
	}
	else {
		ret = 3;
	}
	if (ld < 0 || ud < 0) {
		return MPT_ERROR(BadValue);
	}
	max = da->ivp.neqs * (da->ivp.pint + 1);
	if (ld >= max || ud >= max) {
		return MPT_ERROR(BadValue);
	}
	if (setInt(da, 1, ud) < 0
	 || setInt(da, 0, ld) < 0) {
		return MPT_ERROR(BadOperation);
	}
	da->info[4] = ujac;
	da->info[5] = 1;
	
	return ret;
}


/*!
 * \ingroup mptDaesolvDassl
 * \brief set DASSL property
 * 
 * Assign property of DASSL solver
 * 
 * \param cv   DASSL data
 * \param name name of property to change
 * \param src  data source to change property
 * 
 * \retval <0   failure
 * \retval >=0  used elements
 */
extern int mpt_dassl_set(MPT_SOLVER_STRUCT(dassl) *da, const char *name, MPT_INTERFACE(convertable) *src)
{
	int ret = 0;
	
	if (!name) {
		return MPT_SOLVER_MODULE_FCN(ivp_state)(&da->ivp, &da->t, &da->y, src);
	}
	if (!*name) {
		MPT_IVP_STRUCT(parameters) ivp = MPT_IVPPAR_INIT;
		
		if (src && (ret =  mpt_solver_module_ivpset(&ivp, src)) < 0) {
			return ret;
		}
		mpt_dassl_fini(da);
		mpt_dassl_init(da);
		da->ivp = ivp;
		
		return ret;
	}
	
	if (!strcasecmp(name, "atol")) {
		return mpt_solver_module_tol_set(&da->atol, src, __MPT_IVP_ATOL);
	}
	if (!strcasecmp(name, "rtol")) {
		return mpt_solver_module_tol_set(&da->atol, src, __MPT_IVP_RTOL);
	}
	if (!strncasecmp(name, "jacobian", 3)) {
		return setJacobian(da, src);
	}
	if (!strcasecmp(name, "info3")) {
		int32_t val = 0;
		if (src && (ret = src->_vptr->convert(src, 'i', &val)) < 0) {
			return ret;
		}
		if (val < 0 || val > 1) {
			return MPT_ERROR(BadValue);
		}
		da->info[2] = val;
		return 0;
	}
	if (!strcasecmp(name, "tstop")) {
		double val;
		if (src && (ret = src->_vptr->convert(src, 'd', &val)) < 0) {
			return ret;
		}
		if (!ret) {
			da->info[3] = 0; /* clear max time flag */
			return 0;
		}
		if (setReal(da, 0, val) < 0) {
			return MPT_ERROR(BadOperation);
		}
		da->info[3] = 1;
		return 0;
	}
	if (!strcasecmp(name, "maxstep") || !strcasecmp(name, "hmax")) {
		double val;
		if (src && (ret = src->_vptr->convert(src, 'd', &val)) < 0) {
			return ret;
		}
		if (!ret) {
			da->info[6] = 0; /* clear max time flag */
			return 0;
		}
		if (setReal(da, 1, val) < 0) {
			return MPT_ERROR(BadOperation);
		}
		da->info[6] = 1;
		return 1;
	}
	if (!strcasecmp(name, "h0") || !strcasecmp(name, "initstep") || !strcasecmp(name, "stepinit")) {
		double val;
		if (src && (ret = src->_vptr->convert(src, 'd', &val)) < 0) {
			return ret;
		}
		if (!ret) {
			da->info[7] = 0; /* clear explicit initstep flag */
			return 0;
		}
		if (setReal(da, 2, val) < 0) {
			return MPT_ERROR(BadOperation);
		}
		da->info[7] = 1;
		return 1;
	}
	if (!strcasecmp(name, "maxord")) {
		int32_t val;
		if (src && (ret = src->_vptr->convert(src, 'i', &val)) < 0) {
			return ret;
		}
		if (!ret) {
			da->info[8] = 0; /* clear explicit initstep flag */
			return 0;
		}
		if (val < 1 || val > 5) {
			return MPT_ERROR(BadValue);
		}
		if (setInt(da, 2, val) < 0) {
			return MPT_ERROR(BadOperation);
		}
		da->info[8] = 1;
		return 1;
	}
	if (!strcasecmp(name, "info10") || !strncasecmp(name, "nonnegative", 6)) {
		int32_t val = 0;
		if (src && (ret = src->_vptr->convert(src, 'i', &val)) < 0) {
			return ret;
		}
		if (val < 0 || val > 1) {
			return MPT_ERROR(BadValue);
		}
		da->info[9] = val;
		return ret ? 1 : 0;
	}
	if (!strncasecmp(name, "yprime", 2)) {
		if ((ret = MPT_SOLVER_MODULE_FCN(ivp_vecset)(&da->ivp, &da->yp, src)) < 0) {
			return ret;
		}
		da->info[10] = 1;
		return ret;
	}
	return MPT_ERROR(BadArgument);
}
/*!
 * \ingroup mptDaesolvDassl
 * \brief get DASSL property
 * 
 * Query property of DASSL solver
 * 
 * \param cv   DASSL data
 * \param prop object property
 * 
 * \retval 0   default value
 * \retval <0  failure
 * \retval >0  changed property
 */
extern int mpt_dassl_get(const MPT_SOLVER_STRUCT(dassl) *da, MPT_STRUCT(property) *prop)
{
	const char *name;
	intptr_t pos = -1, id;
	
	if (!prop) {
		return MPT_SOLVER_ENUM(DAE) | MPT_SOLVER_ENUM(PDE);
	}
	if (!(name = prop->name)) {
		if ((pos = (intptr_t) prop->desc) < 0) {
			return MPT_ERROR(BadArgument);
		}
	}
	else if (!*name) {
		prop->name = "dassl";
		prop->desc = "implicit DAE solver with BDF";
		mpt_solver_module_value_ivp(&prop->val, &da->ivp);
		return (da->ivp.neqs == 1 && !da->ivp.pint) ? 0 : 1;
	}
	else if (!strcasecmp(name, "version")) {
		static const char version[] = BUILD_VERSION"\0";
		prop->name = "version";
		prop->desc = "solver release information";
		mpt_solver_module_value_string(&prop->val, version);
		return 0;
	}
	
	id = -1;
	if (name ? !strcasecmp(name, "atol") : pos == ++id) {
		prop->name = "rtol";
		prop->desc = "relative tolerances";
		if (da) {
			return mpt_solver_module_tol_get(&prop->val, &da->atol);
		}
		prop->val.type = 'd';
		prop->val.ptr = &da->atol._d.val;
		return id;
	}
	if (name ? !strcasecmp(name, "rtol") : pos == ++id) {
		prop->name = "atol";
		prop->desc = "absolute tolerances";
		if (da) {
			return mpt_solver_module_tol_get(&prop->val, &da->rtol);
		}
		prop->val.type = 'd';
		prop->val.ptr = &da->rtol._d.val;
		return id;
	}
	if (name ? !strncasecmp(name, "jac", 3) : pos == ++id) {
		const char *type;
		prop->name = "jacobian";
		prop->desc = "(user) jacobian parameters";
		prop->val.type = 's';
		prop->val.ptr = 0;
		if (!da) return id;
		if (da->info[5]) {
			type = da->jac ? "Banded" : "banded";
		} else {
			type = da->jac ? "Full" : "full";
		}
		mpt_solver_module_value_string(&prop->val, type);
		return da->info[5] ? 1 :  0;
	}
	if (name ? !strcasecmp(name, "info3") : pos == ++id) {
		prop->name = "info3";
		prop->desc = "output only at tout";
		prop->val.type = 'i';
		prop->val.ptr = da->info + 2;
		if (!da) return id;
		mpt_solver_module_value_int(&prop->val, da->info + 2);
		return da->info[2] ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "info4") || !strcasecmp(name, "tstop")) : pos == ++id) {
		prop->name = "tstop";
		prop->desc = "do not step past 'tstop'";
		pos = mpt_solver_module_value_rvec(&prop->val, 1, da ? &da->rwork : 0);
		if (!da) return id;
		return da->info[3] ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "info7") || !strcasecmp(name, "hmax")) : pos == ++id) {
		prop->name = "hmax";
		prop->desc = "maximum step size";
		pos = mpt_solver_module_value_rvec(&prop->val, 2, da ? &da->rwork : 0);
		if (!da) return id;
		return da->info[6] ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "info8") || !strcasecmp(name, "h0") || !strcasecmp(name, "initstep")) : pos == ++id) {
		prop->name = "h0";
		prop->desc = "explicit initial step size";
		pos = mpt_solver_module_value_rvec(&prop->val, 3, da ? &da->rwork : 0);
		if (!da) return id;
		return da->info[7] ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "info9") || !strcasecmp(name, "maxord")) : pos == ++id) {
		prop->name = "maxord";
		prop->desc = "maximum order";
		pos = mpt_solver_module_value_ivec(&prop->val, 3, da ? &da->iwork : 0);
		if (!da) return id;
		return da->info[8] ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "info10") || !strncasecmp(name, "nonnegative", 6)) : pos == ++id) {
		prop->name = "nonnegative";
		prop->desc = "restrict to nonnegative solutions";
		prop->val.type = 'i';
		prop->val.ptr = da ? da->info + 9 : 0;
		if (!da) return id;
		mpt_solver_module_value_int(&prop->val, da->info + 9);
		return da->info[9] ? 1 : 0;
	}
	/* state properties */
	if (!name || !da) {
		return MPT_ERROR(BadArgument);
	}
	if (!strncasecmp(name, "yp", 2)) {
		struct iovec *vec;
		prop->name = "yprime";
		prop->desc = "current deviation vector";
		prop->val.type = MPT_type_toVector('d');
		prop->val.ptr = 0;
		if (!da) {
			return id;
		}
		else if (prop->val._bufsize >= sizeof(*vec)) {
			prop->val.ptr = vec = (void *) prop->val._buf;
			vec->iov_base = da->yp;
			vec->iov_len  = da->ivp.neqs * (da->ivp.pint + 1) * sizeof(double);
		}
		return da->info[10] ? 1 : 0;
	}
	return MPT_ERROR(BadArgument);
}
