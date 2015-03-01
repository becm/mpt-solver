
#include <string.h>
#include <errno.h>

#include <sys/resource.h>

#include "array.h"
#include "values.h"

#include "solver.h"

#include "client.h"

/*!
 * \ingroup mptSolver
 * \brief DAE solver step
 * 
 * Execute generic DAE/ODE solver step.
 * 
 * \param gen  solver descriptor
 * \param md   client data
 * \param out  logging descriptor
 * 
 * \return step operation result
 */
extern int mpt_step_ode(MPT_SOLVER_INTERFACE *gen, MPT_SOLVER_STRUCT(data) *md, MPT_INTERFACE(logger) *out)
{
	MPT_INTERFACE_VPTR(Ivp) *ictl = (void *) gen->_vptr;
	struct rusage pre, post;
	double tcurr, *data = 0;
	int err, cont;
	
	if (ictl->step(gen, 0, &tcurr, 0) < 0) {
		(void) mpt_log(out, __func__, MPT_ENUM(LogError), "%s", MPT_tr("solver not prepared for run"));
		errno = EFAULT; return -1;
	}
	/* enshure tend > tcurr */
	if (mpt_iterator_next(md->iter, &tcurr) < 0) {
		(void) mpt_log(out, __func__, MPT_ENUM(LogWarning), "%s", MPT_tr("no iteration steps left"));
		return -2;
	}
	/* initialize current time structures */
	getrusage(RUSAGE_SELF, &pre);
	
	/* indicate single step */
	if ((cont = md->nval) <= 0) {
		MPT_STRUCT(buffer) *buf;
		cont = -cont;
		err = 0;
		if (cont < 2) {
			(void) mpt_log(out, __func__, MPT_ENUM(LogError), "%s: %d", MPT_tr("bad IVP data size"), cont);
		}
		if (!(buf = md->val._buf) || (err = buf->used/sizeof(*data)) < cont) {
			(void) mpt_log(out, __func__, MPT_ENUM(LogError), "%s: %d < %d", MPT_tr("missing IVP data"), err, cont);
			return -1;
		}
		data = (double *) (buf+1);
		cont = 0;
	}
	/* try to complete full run */
	do {
		/* get value destination for ODE/DAE */
		if (cont && !(data = mpt_values_prepare(&md->val, -cont))) {
			(void) mpt_log(out, __func__, MPT_ENUM(LogError), "%s", MPT_tr("unable to get target memory"));
			return -1;
		}
		/* call ODE/DAE solver with current/target time and in/out-data */
		if ((err = ictl->step(gen, data+1, &tcurr, 0)) < 0) {
			(void) mpt_log(out, __func__, MPT_ENUM(LogError), "%s: %d", MPT_tr("IVP step error"), -err);
		}
		/* save time value */
		data[0] = tcurr;
		
		if (out) mpt_solver_status(gen, out);
		
		if (err < 0) break;
		err = 0;
		
	} while (mpt_iterator_next(md->iter, &tcurr) >= 0 && cont);
	
	/* add solver runtime */
	getrusage(RUSAGE_SELF, &post);
	mpt_data_timeradd(md, &pre, &post);
	
	return err;
}

