/*!
 * generic user functions dDASSL solver instance
 */

#include "solver_modfcn.h"

extern int MPT_SOLVER_MODULE_FCN(ivp_values)(const MPT_IVP_STRUCT(parameters) *ivp, double t, const MPT_SOLVER_MODULE_DATA_TYPE *y, const char *desc, MPT_TYPE(PropertyHandler) out, void *usr)
{
	MPT_STRUCT(property) pr;
	struct iovec *vec;
	uint8_t buf[sizeof(t) + 2*sizeof(*vec)];
	size_t len;
	
	pr.name = 0;
	pr.desc = desc ? desc : "";
	pr.val.ptr = &buf;
	
	*((double *) buf) = t;
	vec = (void *) (buf + sizeof(t));
	if (!(len = ivp->pint)) {
		static const char fmt[] = { 'd', MPT_value_toVector('d'), 0 };
		vec->iov_base = (void *) y;
		vec->iov_len  = ivp->neqs * sizeof(*y);
		pr.val.fmt = fmt;
	} else {
		static const char fmt[] = { 'd', MPT_value_toVector('d'), MPT_value_toVector('d'), 0 };
		++len;
		vec->iov_base = ivp->grid;
		vec->iov_len  = len * sizeof(*ivp->grid);
		++vec;
		vec->iov_base = (void *) y;
		vec->iov_len  = len * ivp->neqs * sizeof(*y);
		pr.val.fmt = fmt;
	}
	return out(usr, &pr);
}

