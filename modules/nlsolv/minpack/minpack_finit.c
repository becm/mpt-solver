/*!
 * initialize/free MINPACK instance
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "minpack.h"

extern void mpt_minpack_fini(MPT_SOLVER_STRUCT(minpack) *mpack)
{
	mpt_vecpar_alloc(&mpack->work, 0, 0);
	mpt_vecpar_alloc(&mpack->diag, 0, 0);
}

extern void mpt_minpack_init(MPT_SOLVER_STRUCT(minpack) *mpack)
{
	MPT_NLSPAR_INIT(&mpack->nls);
	
	mpack->work.iov_base = 0;
	mpack->work.iov_len  = 0;
	
	mpack->diag.iov_base = 0;
	mpack->diag.iov_len  = 0;
	
	mpack->solv = 0;   /* select solver according to parameter */
	mpack->mode = 0;
	mpack->info = -1;  /* invalid state */
	mpack->nprint = 0;
	
	mpack->mu = mpack->ml = -1; /* will be set to neqs if not changed */
	
	mpack->maxfev = 0;  /* will be set to 200*(neqs + 1) if not changed */
	
	mpack->nfev = mpack->njev = 0;
	
	mpack->xtol = 0.0;
	mpack->ftol = 1e-7;
	mpack->gtol = 0.0;
	
	mpack->factor = 200.;
	mpack->epsfcn = 0.0;
	
	mpack->fcn.hd = 0;
}

