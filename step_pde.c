
#include <errno.h>

#include <sys/resource.h>

#include "array.h"
#include "values.h"

#include "solver.h"

#include "client.h"

/*!
 * \ingroup mptSolver
 * \brief PDE solver step
 * 
 * Execute generic PDE solver step.
 * 
 * \param gen  solver descriptor
 * \param md   client data
 * \param out  logging descriptor
 * 
 * \return step operation result
 */
extern int mpt_step_pde(MPT_SOLVER_INTERFACE *gen, MPT_SOLVER_STRUCT(data) *md, MPT_INTERFACE(logger) *out)
{
	MPT_INTERFACE_VPTR(Ivp) *ictl = (void *) gen->_vptr;
	MPT_STRUCT(buffer) *val;
	struct rusage pre, post;
	double tcurr, tend, *grid;
	int len, ld, err;
	
	/* get current independent variable from solver */
	if (ictl->step(gen, 0, &tcurr, 0) < 0) {
		(void) mpt_log(out, __func__, MPT_ENUM(LogError), "%s", MPT_tr("solver not prepared for run"));
		errno = EFAULT; return -1;
	}
	/* get PDE grid and data */
	if (!(val = md->val._buf)
	    || (len = md->nval) < 1
	    || (ld  = val->used / len / sizeof(double)) < 2) {
		(void) mpt_log(out, __func__, MPT_ENUM(LogError), "%s", MPT_tr("solver data in invalid state"));
		return -1;
	}
	grid = (double *) (val+1);
	
	/* enshure tend > tcurr */
	if (mpt_iterator_next(md->iter, &tcurr) < 0) {
		(void) mpt_log(out, __func__, MPT_ENUM(LogWarning), "%s", MPT_tr("no iteration steps left"));
		return -2;
	}
	
	/* initialize current time structures */
	getrusage(RUSAGE_SELF, &pre);
	tend = tcurr;
	
	/* execute next PDE step */
	if ((err = ictl->step(gen, grid+len, &tcurr, grid)) < 0) {
		(void) mpt_log(out, __func__, MPT_ENUM(LogError), "%s: %d", MPT_tr("PDE step error"), err);
		return err;
	}
	/* add solver runtime */
	getrusage(RUSAGE_SELF, &post);
	mpt_data_timeradd(md, &pre, &post);
	
	if (out) mpt_solver_status(gen, out);
	
	/* change output dimensions */
	if (err) {
		int ld = md->val._buf->used / sizeof(double) / md->nval;
		md->val._buf->used = (len + err * ld) * sizeof(double);
	}
	
	/* check if further step needed */
	return (tcurr < tend || mpt_iterator_next(md->iter, 0) > 0) ? 1 : 0;
}
