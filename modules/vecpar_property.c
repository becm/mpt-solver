/*!
 * get vecpar properties
 */

#include "solver.h"

static const char name[] = "value";
static const char desc[] = "generic array value";

extern int getValues(MPT_TYPE(dvecpar) *val, MPT_INTERFACE(source) *src)
{
	double	v1 = 0.0, v2, *dest;
	int	len, pos, full;
	
	if (!src) return val->base ? val->d.len/sizeof(double) : 0;
	
	if ((len = src->_vptr->conv(src, 'd', &v1)) < 0)
		return -2;
	
	/* single value only */
	if (!len || (pos = src->_vptr->conv(src, 'd', &v2)) <= 0) {
		dest = mpt_vecpar_alloc((struct iovec *) val, 0, 0);
		val->d.val = v1; return len ? 1 : 0;
	}
	full = (dest = val->base) ? val->d.len/sizeof(double) : 0;
	
	/* initial vector conversion */ 
	if (((pos = 2) > full) && !(dest = mpt_vecpar_alloc((struct iovec *) val, 2, sizeof(double))))
		return -1;
	
	dest[0] = v1;
	dest[1] = v2;
	
	/* read further data */
	while ((len = src->_vptr->conv(src, 'd', &v2)) > 0) {
		/* increase vector space */
		if (pos >= full && !(dest = mpt_vecpar_alloc((struct iovec *) val, pos+1, sizeof(double))))
			break;
		dest[pos++] = v2;
	}
	/* correct used length */
	val->d.len = pos * sizeof(double);
	
	return pos;
}

extern int mpt_vecpar_property(MPT_TYPE(dvecpar) *tol, MPT_STRUCT(property) *pr, MPT_INTERFACE(source) *src)
{
	int len = 0;
	
	if (!pr) {
		if (src && (len = getValues(tol, src)) < 0) return 0;
		return tol->base ? tol->d.len / sizeof(double) : 0;
	}
	/*
	if (!pr->name || *pr->name) {
		return -1;
	}*/
	
	pr->name = name;
	pr->desc = desc;
	
	if (src) len = getValues(tol, src);
	
	if (tol->base) {
		static const char fmt[2] = { (char) (MPT_ENUM(TypeVector) | 'd') };
		pr->data = &tol;
		pr->fmt  = fmt;
	} else {
		static const char fmt[2] = "d";
		pr->data = &tol->d.val;
		pr->fmt  = fmt;
	}
	return len;
}
