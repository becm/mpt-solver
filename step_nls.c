
#include <errno.h>

#include <sys/resource.h>

#include "array.h"

#include "solver.h"

#include "client.h"

/*!
 * \ingroup mptSolver
 * \brief NLS solver step
 * 
 * Execute generic nonlinear solver step.
 * 
 * \param gen  solver descriptor
 * \param dat  client data
 * \param out  logging descriptor
 * 
 * \return step operation result
 */
extern int mpt_step_nls(MPT_SOLVER_INTERFACE *gen, MPT_SOLVER_STRUCT(data) *dat, MPT_INTERFACE(logger) *out)
{
	MPT_INTERFACE_VPTR(Nls) *nctl = (void *) gen->_vptr;
	struct rusage pre, post;
	double *res, *par;
	int err;
	
	if (!(res = mpt_data_grid(dat, 0))) {
		(void) mpt_log(out, __func__, MPT_ENUM(LogError), "%s", MPT_tr("solver data in invalid state"));
		return -1;
	}
	if (!(par = mpt_data_param(dat))) {
		(void) mpt_log(out, __func__, MPT_ENUM(LogError), "%s", MPT_tr("parameter data in invalid state"));
		return -1;
	}
	
	/* initialize current time structures */
	getrusage(RUSAGE_SELF, &pre);
	
	if ((err = nctl->step(gen, par, res)) < 0) {
		(void) mpt_log(out, __func__, MPT_ENUM(LogError), "%s: %d", MPT_tr("NLEQ step error"), err);
	}
	/* add solver runtime */
	getrusage(RUSAGE_SELF, &post);
	mpt_data_timeradd(dat, &pre, &post);
	
	if (out) mpt_solver_status(gen, out);
	
	return err;
}

