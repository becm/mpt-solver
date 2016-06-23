/*!
 * initialize BACOL solver instance
 */

#include <stdlib.h>
#include <string.h>

#include <math.h>

#include "bacol.h"

extern void mpt_bacol_fini(MPT_SOLVER_STRUCT(bacol) *bac)
{
	free(bac->x); bac->x = 0;
	free(bac->y); bac->y = 0;
	
	
	free(bac->ipar.iov_base);
	bac->ipar.iov_base = 0;
	bac->ipar.iov_len  = 0;
	
	free(bac->rpar.iov_base);
	bac->rpar.iov_base = 0;
	bac->rpar.iov_len  = 0;
	
	free(bac->out.wrk.iov_base);
	bac->out.wrk.iov_base = 0;
	bac->out.wrk.iov_len  = 0;
	
	free(bac->out.x); bac->out.x = 0;
	free(bac->out.y); bac->out.y = 0;
	
	mpt_vecpar_cktol(&bac->rtol, 0, 0, __MPT_IVP_RTOL);
	mpt_vecpar_cktol(&bac->atol, 0, 0, __MPT_IVP_ATOL);
	
	bac->mflag.noinit = -1;
	
	switch (bac->_backend) {
#ifdef MPT_BACOL_RADAU
	  case 'r': case 'R':
		free(bac->bd.cpar.iov_base);
		bac->bd.cpar.iov_base = 0;
		bac->bd.cpar.iov_len  = 0;
#endif
		break;
	  default:;
	}
}

extern void mpt_bacol_init(MPT_SOLVER_STRUCT(bacol) *bac)
{
	bac->ivp.neqs = 1;
	bac->ivp.pint = 127;
	
	MPT_VECPAR_INIT(&bac->rtol, __MPT_IVP_RTOL);
	MPT_VECPAR_INIT(&bac->atol, __MPT_IVP_ATOL);
	
	bac->kcol   = 2;
	bac->nint   = 10;
	
	bac->out.nderiv = -1;
	bac->out.nint = 0;
	bac->out.x = 0;
	bac->out.y = 0;
	bac->out.wrk.iov_base = 0;
	bac->out.wrk.iov_len  = 0;
	
	(void) memset(&bac->mflag, 0, sizeof(bac->mflag));
	bac->mflag.noinit = -1;
	
	bac->initstep = NAN;
	
	bac->t = 0.0;
	bac->x = 0;
	bac->y = 0;
	bac->grid = 0;
	
	bac->rpar.iov_base = 0; bac->rpar.iov_len = 0;
	bac->ipar.iov_base = 0; bac->ipar.iov_len = 0;
	
	
	bac->_backend = 0;
	
	memset(&bac->bd, 0, sizeof(bac->bd));
}

