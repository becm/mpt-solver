/*!
 * MPT solver library
 *   create PDE solver parameter from configuration list.
 */

#include <math.h>

#include "meta.h"
#include "node.h"
#include "array.h"
#include "output.h"

#include "values.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief configure PDE data
 * 
 * Get data for PDE solver form configuration.
 * 
 * \param mptr  client data descriptor
 * \param conf  client configuration list
 * \param info  error log descriptor
 * 
 * \return PDE count
 */
extern MPT_INTERFACE(iterator) *mpt_conf_iter(MPT_INTERFACE(metatype) **mptr, MPT_INTERFACE(logger) *info)
{
	MPT_INTERFACE(iterator) *it = 0;
	MPT_INTERFACE(metatype) *mt;
	
	if (!(mt = *mptr)) {
		if (!(mt = mpt_iterator_create(0))) {
			if (info) {
				mpt_log(info, __func__, MPT_LOG(Error), "%s",
				        MPT_tr("no default iterator"));
			}
			return 0;
		}
		*mptr = mt;
		mt->_vptr->conv(mt, MPT_type_pointer(MPT_ENUM(TypeIterator)), &it);
		return it;
		
	}
	/* require valid time source */
	if (mt->_vptr->conv(mt, MPT_type_pointer(MPT_ENUM(TypeIterator)), &it) < 0
	    || !it) {
		MPT_INTERFACE(metatype) *src;
		const char *val;
		
		if (!(val = mpt_meta_data(mt, 0))) {
			if (info) {
				mpt_log(info, __func__, MPT_LOG(Error), "%s",
				        MPT_tr("no iteratior description"));
			}
			return 0;
		}
		if (!(src = mpt_iterator_create(val))) {
			if (info) {
				mpt_log(info, __func__, MPT_LOG(Error), "%s: %s",
				        MPT_tr("bad iteratior description"), val);
			}
			return 0;
		}
		mt->_vptr->ref.unref((void *) mt);
		*mptr = src;
		src->_vptr->conv(src, MPT_type_pointer(MPT_ENUM(TypeIterator)), &it);
		return it;
	}
	/* reset existing iterator */
	if (it->_vptr->reset(it) < 0) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Warning), "%s",
			        MPT_tr("failed to reset time iterator"));
		}
		return 0;
	}
	return it;
}
