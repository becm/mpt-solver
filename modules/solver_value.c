/*!
 * create double vectors of specified length
 */

#include <string.h>

#include <sys/uio.h>

#include "meta.h"

#include "../solver.h"

/*!
 * \ingroup mptSolver
 * \brief set solver value
 * 
 * Convert metatype to solver value source type.
 * 
 * \param val  solver value data
 * \param mt   source metatype
 * 
 * \return conversion result
 */
extern int mpt_solver_value_set(MPT_STRUCT(solver_value) *val, const MPT_INTERFACE(metatype) *mt)
{
	MPT_STRUCT(value) tmp;
	int ret;
	
	if ((ret = mt->_vptr->conv(mt, MPT_ENUM(TypeIterator), &val->_it)) >= 0) {
		val->_val.fmt = 0;
		val->_val.ptr = 0;
		if (!val->_it) {
			return 0;
		}
		return MPT_ENUM(TypeIterator);
	}
	if ((ret = mt->_vptr->conv(mt, MPT_ENUM(TypeValue), &tmp)) >= 0) {
		if (tmp.fmt && !tmp.ptr) {
			return MPT_ERROR(BadValue);
		}
		val->_it = 0;
		val->_val = tmp;
		return MPT_ENUM(TypeValue);
	}
	if ((ret = mt->_vptr->conv(mt, 's', &val->_val.ptr)) >= 0) {
		val->_it = 0;
		val->_val.fmt = 0;
		return 's';
	}
	return MPT_ERROR(BadType);
}
static int solverNext(MPT_STRUCT(solver_value) *val, void *dest, int type, size_t len)
{
	MPT_INTERFACE(iterator) *it;
	int ret;
	if ((it = val->_it)) {
		union {
			double d;
			uintptr_t k;
			uint32_t u;
			int32_t i;
		} tmp;
		int next;
		if ((ret = it->_vptr->get(it, type, &tmp)) < 0) {
			return ret;
		}
		if ((next = it->_vptr->advance(it)) < 0) {
			return next;
		}
		if (dest && len) {
			memcpy(dest, &tmp, len);
		}
		return ret;
	}
	if (val->_val.fmt) {
		uint8_t fmt;
		if (!(fmt = *val->_val.fmt)) {
			return 0;
		}
		if (fmt != type) {
			MPT_INTERFACE(metatype) *mt;
			if (fmt != MPT_ENUM(TypeMeta)) {
				return MPT_ERROR(BadType);
			}
			if (!(mt = *((MPT_INTERFACE(metatype) **) val->_val.ptr))) {
				return MPT_ERROR(BadType);
			}
			if ((ret = mt->_vptr->conv(mt, type, dest)) < 0) {
				return ret;
			}
			len = sizeof(mt);
		}
		else if (dest) {
			memcpy(dest, val->_val.ptr, len);
		}
		val->_val.ptr = ((uint8_t *) val->_val.ptr) + len;
		return fmt;
	}
	return MPT_ERROR(BadType);
}

/*!
 * \ingroup mptSolver
 * \brief get key value
 * 
 * Convert solver value element to key id.
 * 
 * \param val  solver value data
 * \param ley  key value
 * 
 * \return conversion result
 */
extern int mpt_solver_next_double(MPT_STRUCT(solver_value) *val, double *d)
{
	return solverNext(val, d, 'd', sizeof(*d));
}
/*!
 * \ingroup mptSolver
 * \brief get key value
 * 
 * Convert solver value element to key id.
 * 
 * \param val  solver value data
 * \param ley  key value
 * 
 * \return conversion result
 */
extern int mpt_solver_next_key(MPT_STRUCT(solver_value) *val, uintptr_t *key)
{
	int ret;
	const char *tmp;
	if ((ret =  solverNext(val, &tmp, 'k', sizeof(tmp))) < 0) {
		return ret;
	}
	if (ret && tmp && key) {
		*key = tmp[0];
	}
	return ret;
}

/*!
 * \ingroup mptSolver
 * \brief get key value
 * 
 * Convert solver value element to key id.
 * 
 * \param val  solver value data
 * \param ley  key value
 * 
 * \return conversion result
 */
extern int mpt_solver_next_uint(MPT_STRUCT(solver_value) *val, uint32_t *u)
{
	return solverNext(val, u, 'u', sizeof(*u));
}
/*!
 * \ingroup mptSolver
 * \brief get key value
 * 
 * Convert solver value element to key id.
 * 
 * \param val  solver value data
 * \param ley  key value
 * 
 * \return conversion result
 */
extern int mpt_solver_next_int(MPT_STRUCT(solver_value) *val, int32_t *i)
{
	return solverNext(val, i, 'i', sizeof(*i));
}
