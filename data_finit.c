/*!
 * initialize/finalize solver data descriptor.
 */

#include <string.h>
#include <errno.h>

#include "array.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief clear client data
 * 
 * Reset client data to default values.
 * 
 * \param md  client data descriptor
 */
extern void mpt_data_clear(MPT_SOLVER_STRUCT(data) *md)
{
	MPT_STRUCT(buffer) *buf;
	MPT_INTERFACE(metatype) *mt;
	
	if ((buf = md->param._buf)) {
		buf->used = 0;
	}
	if ((buf = md->val._buf)) {
		buf->used = 0;
	}
	if ((mt = (void *) md->iter)) {
		mt->_vptr->unref(mt);
		md->iter = 0;
	}
	md->npar = 0;
	md->nval = 0;
	
	memset(md->mask, 0, sizeof(md->mask));
	
	memset(&md->ru_usr, 0, sizeof(md->ru_usr));
	memset(&md->ru_sys, 0, sizeof(md->ru_sys));
}
/*!
 * \ingroup mptSolver
 * \brief terminate client data
 * 
 * Clear client data resources.
 * 
 * \param md  client data descriptor
 */
extern void mpt_data_fini(MPT_SOLVER_STRUCT(data) *md)
{
	MPT_INTERFACE(metatype) *mt;
	
	if ((mt = (void *) md->iter)) {
		mt->_vptr->unref(mt);
		md->iter = 0;
	}
	mpt_array_clone(&md->param, 0);
	mpt_array_clone(&md->val, 0);
}
/*!
 * \ingroup mptSolver
 * \brief init client data
 * 
 * Initialize raw data to client data default values.
 * 
 * \param md  client data descriptor
 */
extern void mpt_data_init(MPT_SOLVER_STRUCT(data) *md)
{
	memset(md, 0, sizeof(*md));
}

