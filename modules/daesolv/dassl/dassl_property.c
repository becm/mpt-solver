/*!
 * set dDASSL integrator parameters
 */

#include <string.h>
#include <strings.h>
#include <errno.h>

#include "version.h"

#include "dassl.h"

static int setInt(MPT_SOLVER_STRUCT(dassl) *da, size_t pos, int val)
{
	int *iwk = da->iwork.iov_base;
	size_t size = da->iwork.iov_len / sizeof(int);
	if (pos >= size) {
		if (!val) {
			return 0;
		}
		if (!(iwk = mpt_vecpar_alloc(&da->iwork, pos + 1, sizeof(int)))) {
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
		if (!(rwk = mpt_vecpar_alloc(&da->rwork, pos + 1, sizeof(double)))) {
			return MPT_ERROR(BadOperation);
		}
	}
	rwk[pos] = val;
	return 1;
}

static int setJacobian(MPT_SOLVER_STRUCT(dassl) *da, MPT_INTERFACE(metatype) *src)
{
	char *key;
	int32_t ld, ud;
	int ret, len, ujac, max;
	
	if (!src) {
		da->info[4] = 1;
		da->info[5] = 0;
		return 0;
	}
	if ((len = src->_vptr->conv(src, 'k' | MPT_ENUM(ValueConsume), &key)) < 0) {
		return len;
	}
	if (!len || !key) {
		da->info[4] = 1;
		da->info[5] = 0;
		return 0;
	}
	
	switch (key[0]) {
		case 'F': da->info[4] = 1; da->info[5] = 0; return 1;
		case 'f': da->info[4] = 0; da->info[5] = 0; return 1;
		case 'B': ujac = 1; break;
		case 'b': ujac = 0; break;
		default:
			return MPT_ERROR(BadValue);
	}
	len = 1;
	if ((ret = src->_vptr->conv(src, 'i' | MPT_ENUM(ValueConsume), &ld) < 0)) {
		return ret;
	}
	max = da->ivp.neqs * (da->ivp.pint + 1);
	if (!ret) {
		ld = da->ivp.neqs;
		ud = ld;
	}
	else if (ld < 0 || ld > max) {
		return MPT_ERROR(BadValue);
	}
	else if (src->_vptr->conv(src, 'i' | MPT_ENUM(ValueConsume), &ud) < 0) {
		return ret;
	}
	else if (!ret) {
		ud = ld;
		len = 2;
	}
	else if (ud < 0 || ud > max) {
		return MPT_ERROR(BadValue);
	}
	else {
		len = 3;
	}
	
	if (setInt(da, 1, ud) < 0 || setInt(da, 0, ld) < 0) {
		return MPT_ERROR(BadOperation);
	}
	da->info[4] = ujac;
	da->info[5] = 1;
	
	return len;
}


/*!
 * \ingroup mptSundialsCVode
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
extern int mpt_dassl_set(MPT_SOLVER_STRUCT(dassl) *da, const char *name, MPT_INTERFACE(metatype) *src)
{
	int ret = 0;
	
	if (!name) {
		double t = da->t;
		size_t required;
		
		if (src && (ret = src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), &t) <= 0)) {
			if (ret < 0) {
				return ret;
			}
			src = 0;
		}
		required = da->ivp.neqs * (da->ivp.pint + 1);
		if ((ret = mpt_vecpar_set(&da->y, required, src)) < 0) {
			return ret;
		}
		da->t = t;
		
		return src ? ++ret : 0;
	}
	if (!*name) {
		MPT_SOLVER_STRUCT(ivppar) ivp = da->ivp;
		
		if (src && (ret =  mpt_ivppar_set(&ivp, src)) < 0) {
			return ret;
		}
		mpt_dassl_fini(da);
		mpt_dassl_init(da);
		
		da->ivp = ivp;
		
		return ret;
	}
	
	if (!strcasecmp(name, "atol")) {
		return mpt_vecpar_settol(&da->atol, src, __MPT_IVP_ATOL);
	}
	if (!strcasecmp(name, "rtol")) {
		return mpt_vecpar_settol(&da->atol, src, __MPT_IVP_RTOL);
	}
	if (!strncasecmp(name, "jacobian", 3)) {
		return setJacobian(da, src);
	}
	if (!strcasecmp(name, "info3")) {
		int32_t val = 0;
		if (src && (ret = src->_vptr->conv(src, 'i' | MPT_ENUM(ValueConsume), &val)) < 0) return ret;
		if (val < 0 || val > 1) return MPT_ERROR(BadValue);
		da->info[2] = val;
		return ret ? 1 : 0;
	}
	if (!strcasecmp(name, "tstop")) {
		double val;
		if (src && (ret = src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), &val)) < 0) return ret;
		if (!ret) {
			da->info[3] = 0; /* clear max time flag */
			return 0;
		}
		if (setReal(da, 0, val) < 0) {
			return MPT_ERROR(BadOperation);
		}
		da->info[3] = 1;
		return 1;
	}
	if (!strcasecmp(name, "maxstep") || !strcasecmp(name, "hmax")) {
		double val;
		if (src && (ret = src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), &val)) < 0) return ret;
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
		if (src && (ret = src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), &val)) < 0) return ret;
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
		if (src && (ret = src->_vptr->conv(src, 'i' | MPT_ENUM(ValueConsume), &val)) < 0) return ret;
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
	if (!strcasecmp(name, "info10") || strncasecmp(name, "nonnegative", 6)) {
		int32_t val = 0;
		if (src && (ret = src->_vptr->conv(src, 'i' | MPT_ENUM(ValueConsume), &val)) < 0) return ret;
		if (val < 0 || val > 1) {
			return MPT_ERROR(BadValue);
		}
		da->info[9] = val;
		return ret ? 1 : 0;
	}
	if (!strncasecmp(name, "yprime", 2)) {
		if (!src) {
			ret = -1;
		} else {
			ret = da->ivp.neqs * (da->ivp.pint + 1);
		}
		if ((ret = mpt_vecpar_set(&da->yp, ret, src)) < 0) {
			return ret;
		}
		da->info[10] = 1;
		return ret;
	}
	return MPT_ERROR(BadArgument);
}

/*!
 * \ingroup mptDaesolvDassl
 * \brief set DASSL property
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
		return MPT_ENUM(TypeSolver);
	}
	if (!(name = prop->name)) {
		if ((pos = (intptr_t) prop->desc) < 0) {
			return MPT_ERROR(BadArgument);
		}
	}
	else if (!*name) {
		prop->name = "dassl"; prop->desc = "implicit DAE solver with BDF";
		prop->val.fmt = "iid"; prop->val.ptr = &da->ivp;
		return MPT_SOLVER_ENUM(ODE) | MPT_SOLVER_ENUM(DAE) | MPT_SOLVER_ENUM(PDE);
	}
	else if (!strcasecmp(name, "version")) {
		static const char version[] = BUILD_VERSION"\0";
		prop->name = "version"; prop->desc = "solver release information";
		prop->val.fmt = 0; prop->val.ptr = version;
		return 0;
	}
	
	id = -1;
	if (name ? !strcasecmp(name, "atol") : pos == ++id) {
		if (!da) { prop->val.fmt = "d"; prop->val.ptr = &da->rtol.d.val; }
		else { id = mpt_vecpar_get(&da->rtol, &prop->val); }
		prop->name = "rtol"; prop->desc = "relative tolerances";
		return id;
	}
	if (name ? !strcasecmp(name, "rtol") : pos == ++id) {
		if (!da) { prop->val.fmt = "d"; prop->val.ptr = &da->rtol.d.val; }
		else { id = mpt_vecpar_get(&da->rtol, &prop->val); }
		prop->name = "atol"; prop->desc = "absolute tolerances";
		return id;
	}
	if (!strncasecmp(name, "jac", 3)) {
		size_t len = da ? da->iwork.iov_len / sizeof(int) : 0;
		prop->name = "jacobian"; prop->desc = "(user) jacobian parameters";
		prop->val.fmt = "ii"; prop->val.ptr = 0;
		if (!da) return id;
		if (len > 1) prop->val.ptr = da->iwork.iov_base;
		return da->info[5] ? 1 :  0;
	}
	if (name ? !strcasecmp(name, "info3") : pos == ++id) {
		prop->name = "info3"; prop->desc = "output only at tout";
		prop->val.fmt = "i"; prop->val.ptr = da ? da->info+2 : 0;
		if (!da) return id;
		return da->info[2] ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "info4") || !strcasecmp(name, "tstop")) : pos == ++id) {
		size_t len = da ? da->rwork.iov_len / sizeof(double) : 0;
		prop->name = "tstop"; prop->desc = "do not step past 'tstop'";
		prop->val.fmt = "d"; prop->val.ptr = len ? da->rwork.iov_base : 0;
		if (!da) return id;
		return da->info[3] ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "info7") || !strcasecmp(name, "hmax")) : pos == ++id) {
		size_t len = da ? da->rwork.iov_len / sizeof(double) : 0;
		prop->name = "hmax"; prop->desc = "maximum step size";
		prop->val.fmt = "d"; prop->val.ptr = len > 1 ? ((double *) da->rwork.iov_base) + 1 : 0;
		if (!da) return id;
		return da->info[6] ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "info8") || !strcasecmp(name, "h0") || !strcasecmp(name, "initstep")) : pos == ++id) {
		size_t len = da ? da->rwork.iov_len / sizeof(double) : 0;
		prop->name = "h0"; prop->desc = "explicit initial step size";
		prop->val.fmt = "d"; prop->val.ptr = len > 2 ? ((double *) da->rwork.iov_base) + 2 : 0;
		if (!da) return id;
		return da->info[7] ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "info9") || !strcasecmp(name, "maxord")) : pos == ++id) {
		size_t len = da ? da->iwork.iov_len / sizeof(double) : 0;
		prop->name = "maxord"; prop->desc = "maximum order";
		prop->val.fmt = "i"; prop->val.ptr = len > 8 ? ((double *) da->rwork.iov_base) + 8 : 0;
		if (!da) return id;
		return da->info[8] ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "info10") || !strncasecmp(name, "nonnegative", 6)) : pos == ++id) {
		prop->name = "info10"; prop->desc = "restrict to nonnegative solutions";
		prop->val.fmt = "i"; prop->val.ptr = da ? da->info + 9 : 0;
		if (!da) return id;
		return da->info[9] ? 1 : 0;
	}
	if (name ? !strncasecmp(name, "yp", 2) : pos == ++id) {
		prop->name = "yprime"; prop->desc = "deviation at current time";
		prop->val.fmt = "d"; prop->val.ptr = da ? da->yp : 0;
		return da->info[10] ? da->ivp.neqs * (da->ivp.pint + 1) : 0;
	}
	return MPT_ERROR(BadArgument);
}
