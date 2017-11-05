/*!
 * set metatype data.
 */

#include <stdlib.h>

#include "meta.h"
#include "convert.h"

#include "object.h"

struct wrapValue
{
	MPT_INTERFACE(metatype) _mt;
	double val;
};

static void valueUnref(MPT_INTERFACE(reference) *ref)
{
	(void) ref;
}
static uintptr_t valueRef(MPT_INTERFACE(reference) *ref)
{
	(void) ref;
	return 0;
}
static int valueConv(const MPT_INTERFACE(metatype) *mt, int type, void *ptr)
{
	const struct wrapValue *v = (void *) mt;
	
	if (!type) {
		static const char fmt[] = { 'd', 'f', 0 };
		if (ptr) *((const char **) ptr) = fmt;
		return MPT_ENUM(TypeMeta);
	}
#ifdef _MPT_FLOAT_EXTENDED_H
	if (type == 'e') {
		if (ptr) *((long double *) ptr) = v->val;
		return MPT_ENUM(TypeMeta);
	}
#endif
	if (type == 'd') {
		if (ptr) *((double *) ptr) = v->val;
		return MPT_ENUM(TypeMeta);
	}
	if (type == 'f') {
		if (ptr) *((float *) ptr) = v->val;
		return MPT_ENUM(TypeMeta);
	}
	return MPT_ERROR(BadType);
}
static MPT_INTERFACE(metatype) *valueClone(const MPT_INTERFACE(metatype) *mt);

static const MPT_INTERFACE_VPTR(metatype) valueMetaCtl = {
	{ valueUnref, valueRef },
	valueConv,
	valueClone
};
static MPT_INTERFACE(metatype) *valueClone(const MPT_INTERFACE(metatype) *mt)
{
	struct wrapValue *cp, *v = (void *) mt;
	if (!(cp = malloc(sizeof(*cp)))) {
		return 0;
	}
	cp->_mt._vptr = &valueMetaCtl;
	cp->val = v->val;
	
	return &cp->_mt;
}

/*!
 * \ingroup mptSolver
 * \brief ODE steps
 * 
 * Execute generic DAE/ODE solver steps.
 * 
 * \param obj  object interface descriptor
 * \param prop property name
 * \param val  floating point value to assign
 * 
 * \return step operation result
 */
extern int mpt_solver_setvalue(MPT_INTERFACE(object) *obj, const char *prop, double val)
{
	struct wrapValue v;
	
	v._mt._vptr = &valueMetaCtl;
	v.val = val;
	
	return obj->_vptr->setProperty(obj, prop, &v._mt);
}
