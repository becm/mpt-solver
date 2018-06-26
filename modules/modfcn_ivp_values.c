/*!
 * MPT solver module helper function
 *   dispatch IVP state data via property handler
 */

#include "solver_modfcn.h"

extern int MPT_SOLVER_MODULE_FCN(ivp_values)(const MPT_IVP_STRUCT(parameters) *ivp, double t, const MPT_SOLVER_MODULE_DATA_TYPE *y, const char *desc, MPT_TYPE(property_handler) out, void *usr)
{
	MPT_STRUCT(property) pr;
	struct iovec *vec;
	uint8_t buf[sizeof(t) + 2 * sizeof(*vec)];
	size_t len;
	
	pr.name = 0;
	pr.desc = desc ? desc : "";
	pr.val.ptr = &buf;
	
	*((double *) buf) = t;
	vec = (void *) (buf + sizeof(t));
	if (!(len = ivp->pint)) {
		static const uint8_t fmt[] = {
			'd',
			MPT_type_vector(MPT_SOLVER_MODULE_DATA_ID),
			0
		};
		vec->iov_base = (void *) y;
		vec->iov_len  = ivp->neqs * sizeof(*y);
		pr.val.fmt = fmt;
	} else {
		static const uint8_t fmt[] = {
			'd',
			MPT_type_vector('d'),
			MPT_type_vector(MPT_SOLVER_MODULE_DATA_ID),
			0
		};
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

