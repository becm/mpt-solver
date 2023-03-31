/*!
 * MPT solver library
 *   set object property
 */

#include <stdlib.h>

#include "solver.h"

static int valueConv(MPT_INTERFACE(convertable) *val, MPT_TYPE(value) type, void *ptr)
{
	const double *v = (void *) (val + 1);
	
	if (!type) {
		static const char fmt[] = { 'd', 'f', 0 };
		if (ptr) *((const char **) ptr) = fmt;
		return ptr ? 0 : 'd';
	}
#ifdef _MPT_FLOAT_EXTENDED_H
	if (type == 'e') {
		if (ptr) *((long double *) ptr) = *v;
		return 'd';
	}
#endif
	if (type == 'd') {
		if (ptr) *((double *) ptr) = *v;
		return 'd';
	}
	if (type == 'f') {
		if (ptr) *((float *) ptr) = *v;
		return 'd';
	}
	return MPT_ERROR(BadType);
}

/*!
 * \ingroup mptSolver
 * \brief set single float value
 * 
 * Set object property to single float value.
 * 
 * \param obj  object interface descriptor
 * \param prop property name
 * \param val  floating point value to assign
 * 
 * \return assign operation result
 */
extern int mpt_solver_setvalue(MPT_INTERFACE(object) *obj, const char *prop, double val)
{
	static const MPT_INTERFACE_VPTR(convertable) valueConvCtl = { valueConv };
	struct
	{
		MPT_INTERFACE(convertable) _ctl;
		double val;
	} v;
	
	v._ctl._vptr = &valueConvCtl;
	v.val = val;
	
	return obj->_vptr->set_property(obj, prop, &v._ctl);
}
