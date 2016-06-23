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
extern int mpt_clientdata_assign(MPT_SOLVER_STRUCT(clientdata) *cd, const MPT_STRUCT(value) *val)
{
	MPT_INTERFACE(output) *out;
	
	if (!val) {
		out = cd->out;
		if (!out || cd->log || !out->_vptr->obj.addref((void *) out)) {
			return 0;
		}
		if ((cd->log = mpt_output_logger(out))) {
			return 1;
		}
		out->_vptr->obj.unref((void *) out);
		return MPT_ERROR(BadOperation);
	}
	if (!val->fmt) {
		if ((out = cd->out)) {
			return mpt_object_pset((void *) out, 0, val, 0);
		}
		return 0;
	}
	if (val->fmt[0] == MPT_ENUM(TypeSolver)) {
		MPT_SOLVER_STRUCT(generic) *sol = 0;
		if (val->ptr && (sol = *((void **) val->ptr))) {
			if (!sol->_vptr->obj.addref((void *) sol)) {
				return MPT_ERROR(BadOperation);
			}
		}
		if (cd->pr._ref) {
			((MPT_STRUCT(metatype) *) cd->pr._ref)->_vptr->unref(cd->pr._ref);
		}
		cd->pr._types[0] = MPT_ENUM(TypeSolver);
		cd->pr._types[1] = 0;
		
		return 1;
	}
	if (val->fmt[0] != MPT_ENUM(TypeOutput)) {
		return MPT_ERROR(BadType);
	}
	out = 0;
	if (val->ptr && (out = *((void **) val->ptr))) {
		if (!out->_vptr->obj.addref((void *) out)) {
			return MPT_ERROR(BadOperation);
		}
		if (!cd->log && out->_vptr->obj.addref((void *) out)) {
			if (!(cd->log = mpt_output_logger(out))) {
				out->_vptr->obj.unref((void *) out);
			}
		}
	}
	if (cd->out) {
		cd->out->_vptr->obj.unref((void *) cd->out);
	}
	cd->out = out;
	
	return 1;
}
