/*!
 * initialize MEBDFI instance
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "mebdfi.h"

extern void mpt_mebdfi_fini(MPT_SOLVER_STRUCT(mebdfi) *me)
{
	if (me->y) {
		free(me->y);
		me->y = 0;
	}
	if (me->yp) {
		free(me->yp);
		me->yp = 0;
	}
	if (me->dmas) {
		free(me->dmas);
		me->dmas = 0;
	}
	mpt_vecpar_alloc(&me->rwork, 0, 0);
	mpt_vecpar_alloc(&me->iwork, 0, 0);
	
	mpt_vecpar_cktol(&me->rtol, 0, 0, __MPT_IVP_RTOL);
	mpt_vecpar_cktol(&me->atol, 0, 0, __MPT_IVP_ATOL);
	
	me->state = -1;
}

extern void mpt_mebdfi_init(MPT_SOLVER_STRUCT(mebdfi) *me)
{
	MPT_IVPPAR_INIT(&me->ivp);
	
	me->t = 0.;
	me->h = 0.;
	
	me->y = 0;
	me->yp = 0;
	me->dmas = 0;
	
	/* allocate inital space for parameters */
	me->rwork.iov_base = 0; me->rwork.iov_len = 0;
	me->iwork.iov_base = 0; me->iwork.iov_len = 0;
	
	MPT_VECPAR_INIT(&me->rtol, __MPT_IVP_RTOL);
	MPT_VECPAR_INIT(&me->atol, __MPT_IVP_ATOL);
	
	
	me->jnum   = 0;  /* allow analytical jacobian */
	me->jbnd   = 0;  /* jacobian is banded */
	me->type   = 0;  /* normal step strategy */
	me->state  = -1; /* invalid initial state */
	
	me->lout   = 6;  /* FORTRAN stdout */
	me->maxder = 7;  /* default value */
	
	(void) memset(me->mbnd, 0,sizeof(me->mbnd));
	
	me->rpar = 0;
	me->ipar = 0;
	
	me->fcn = 0;
	me->jac = 0;
	
	me->state = -1;
}

