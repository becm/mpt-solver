/*!
 * MPT solver module helper function
 *   prepare vecpar data
 */

#include <string.h>

#include "meta.h"

#include "../solver.h"

/*!
 * \ingroup mptSolver
 * \brief consume iterator value
 * 
 * Use opbect interface to nest multiple properties in single value.
 * 
 * \param it    iteratable data source
 * \param type  target type
 * \param ptr   target address
 * \param len   size of target value for raw copy
 * 
 * \return result of iterator advance operation
 */
extern int mpt_solver_module_consume_value(MPT_INTERFACE(iterator) *it, MPT_TYPE(type) type, void *ptr, size_t len)
{
	const MPT_STRUCT(value) *val;
	MPT_INTERFACE(convertable) *src;
	int ret;
	
	if (!it || !(val = it->_vptr->value(it))) {
		return MPT_ERROR(MissingData);
	}
	if (len && val->_type == type) {
		if (ptr) {
			if (val->_addr) {
				memcpy(ptr, val->_addr, len);
			} else {
				memset(ptr, 0, len);
			}
		}
		return it->_vptr->advance(it);
	}
	if (!MPT_type_isConvertable(val->_type)) {
		return MPT_ERROR(BadType);
	}
	if (!(src = *((void * const *) val->_addr))) {
		return MPT_ERROR(MissingData);
	}
	if ((ret = src->_vptr->convert(src, type, ptr)) < 0) {
		return ret;
	}
	return it->_vptr->advance(it);
}
