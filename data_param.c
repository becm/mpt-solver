/*!
 * get parameter start and dimension.
 */

#include <errno.h>

#include "node.h"
#include "array.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief get parameter data
 * 
 * Get parameter address from client data.
 * 
 * \param md  client data descriptor
 * 
 * \return start of parameters
 */
extern double *mpt_data_param(MPT_SOLVER_STRUCT(data) *md)
{
	MPT_STRUCT(buffer) *buf;
	int len;
	
	/* parameters are initialized */
	if ((len = md->npar) <= 0 || !(buf = md->param._buf)) return 0;
	if ((len = buf->used/sizeof(double)) < md->npar) {
		if (!(mpt_array_slice(&md->param, buf->used, md->npar*sizeof(double))))
			return 0;
		for (; len < md->npar; ++len)
			((double *) (buf+1))[len] = 0;
	}
	return (double *) (buf+1);
}
