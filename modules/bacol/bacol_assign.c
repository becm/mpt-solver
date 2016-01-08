/*!
 * create output values for BACOL descriptor
 */

#include <stdlib.h>
#include <string.h>

#include "bacol.h"

extern int mpt_bacol_assign(MPT_SOLVER_STRUCT(bacol) *bac, const MPT_STRUCT(value) *ptr)
{
	MPT_STRUCT(value) val;
	const struct iovec *vec;
	double t, *x, *y;
	size_t l;
	uint oint;
	int neqs;
	
	neqs = bac->ivp.neqs;
	
	if (neqs < 1 || bac->mflag.noinit) {
		return MPT_ERROR(BadArgument);
	}
	/* initialize with default values */
	if (!ptr || bac->mflag.noinit >= 0) {
		return MPT_ERROR(BadArgument);
	}
	if (!ptr->fmt) {
		return MPT_ERROR(BadType);
	}
	val = *ptr;
	
	/* set time step */
	if (*val.fmt == 'd') {
		const double *tp = val.ptr;
		++val.fmt;
		val.ptr = tp + 1;
		t = *tp;
	} else {
		t = 0.0;
	}
	/* need grid data */
	if (*val.fmt != MPT_value_toVector('d')) {
		return MPT_ERROR(BadType);
	}
	vec = val.ptr;
	x = vec->iov_base;
	l = vec->iov_len;
	
	++val.fmt;
	++vec;
	
	/* grid setup */
	if (!*val.fmt) {
		uint nint;
		/* need matching grid dimensions */
		if ((nint = bac->nint)
		    && l != ((bac->nint+1) * sizeof(double))) {
			return MPT_ERROR(BadValue);
		}
		if (l > ((bac->ivp.pint+1) * sizeof(double))) {
			return MPT_ERROR(BadValue);
		}
		bac->nint = l / sizeof(double);
		l = bac->nint * sizeof(double);
		
		if (mpt_bacol_prepare(bac) < 0) {
			bac->nint = nint;
			return MPT_ERROR(BadOperation);
		}
		if (x) {
			memcpy(bac->x, x, (bac->nint+1) * sizeof(*x));
		}
		bac->t = t;
		return 2;
	}
	/* initialize output data */
	if (*val.fmt++ != MPT_value_toVector('d') || *val.fmt) {
		return MPT_ERROR(BadType);
	}
	if ((oint = bac->out.nint)) {
		if (l != ((oint+1) * sizeof(double))) {
			return MPT_ERROR(BadValue);
		}
	} else {
		bac->out.nint = (l / sizeof(double)) - 1;
	}
	y = vec->iov_base;
	l = vec->iov_len;
	
	if (mpt_bacol_values(bac, 0, -1, 0) < 0) {
		bac->out.nint = oint;
		return MPT_ERROR(BadOperation);
	}
	oint = bac->out.nint;
	if (x) {
		memcpy(bac->out.x, x, (oint+1) * sizeof(*x));
	} else {
		double dx = 1.0 / oint;
		uint i;
		x = bac->x;
		for (i = 0; i <= oint; ++i) x[i] = i * dx;
	}
	if (l) {
		if (y) {
			memcpy(bac->out.y, y, l);
		} else {
			memset(bac->out.y, 0, l);
		}
	}
	bac->t = t;
	return 2;
}


