/*!
 * solver client data operations
 */

#include <string.h>

#include "client.h"
#include "message.h"

#include "solver.h"

struct _unrefInstance
{
	void (*unref)(void *);
};
/*!
 * \ingroup mptSolver
 * \brief set solver client data
 * 
 * Assign element in solver client data.
 * 
 * \param cd  data pointer
 * \param val source value
 * 
 * \return event registration hint or error
 */
void mpt_clientdata_fini(MPT_SOLVER_STRUCT(clientdata) *cd)
{
	struct {
		struct _unrefInstance *_vptr;
	} *ptr;
	if ((ptr = (void*) cd->out)) {
		ptr->_vptr->unref(ptr);
	}
	if ((ptr = (void*) cd->log)) {
		ptr->_vptr->unref(ptr);
	}
	if ((ptr = cd->pr._ref)) {
		ptr->_vptr->unref(ptr);
	}
	memset(cd, 0, sizeof(*cd));
}
/*!
 * \ingroup mptSolver
 * \brief set solver client data
 * 
 * Assign element in solver client data.
 * 
 * \param cd  data pointer
 * \param val source value
 * 
 * \return event registration hint or error
 */
int mpt_clientdata_assign(MPT_SOLVER_STRUCT(clientdata) *cd, const MPT_STRUCT(value) *val)
{
	MPT_INTERFACE(object) *obj;
	MPT_INTERFACE(logger) *log;
	
	if (!val) {
		obj = (void *) cd->out;
		log = cd->out ? mpt_object_logger(obj) : 0;
		
		if (log && !cd->log && obj->_vptr->addref(obj)) {
			cd->log = log;
			return 1;
		}
		cd->_outlog = log;
		return 0;
	}
	if (val->fmt[0] == MPT_ENUM(TypeOutput)) {
		obj = *((void **) val->ptr);
		if (obj && !obj->_vptr->addref(obj)) {
			return MPT_ERROR(BadOperation);
		}
		if (cd->out) {
			cd->out->_vptr->obj.unref((void *) cd->out);
		}
		cd->out = (void *) obj;
		cd->_outlog = mpt_object_logger(obj);
		return 1;
	}
	if (val->fmt[0] == MPT_ENUM(TypeObject)) {
		obj = *((void **) val->ptr);
		if (!obj
		    || !(log = mpt_object_logger(obj))
		    || !obj->_vptr->addref((void *) obj)) {
			return MPT_ERROR(BadType);
		}
		if (cd->log) {
			cd->log->_vptr->unref(cd->log);
		}
		cd->log = log;
		return 1;
	}
	if (val->fmt[0] == MPT_ENUM(TypeLogger)) {
		log = *((void **) val->ptr);
		if (!log && cd->log) {
			cd->log->_vptr->unref(cd->log);
			cd->log = 0;
		}
		cd->_outlog = log;
		return 0;
	}
	return MPT_ERROR(BadValue);
}
