/*!
 * initialize BACOL solver instance
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "bacol.h"

extern void mpt_bacol_fini(MPT_SOLVER_STRUCT(bacol) *data)
{
	if (data->x) free(data->x);
	if (data->y) free(data->y);
	
	data->y = data->x = 0;
	
	if (data->ipar.iov_base) {
		free(data->ipar.iov_base);
		data->ipar.iov_base = 0;
		data->ipar.iov_len  = 0;
	}
	if (data->rpar.iov_base) {
		free(data->rpar.iov_base);
		data->rpar.iov_base = 0;
		data->rpar.iov_len  = 0;
	}
	if (data->owrk.iov_base) {
		free(data->owrk.iov_base);
		data->owrk.iov_base = 0;
		data->owrk.iov_len  = 0;
	}
	
	mpt_vecpar_cktol(&data->rtol, 0, 0, __MPT_IVP_RTOL);
	mpt_vecpar_cktol(&data->atol, 0, 0, __MPT_IVP_ATOL);
	
	data->mflag.noinit = -1;
	
	switch (data->backend) {
#ifdef MPT_BACOL_RADAU
	  case 'r': case 'R':
		free(data->bd.cpar.iov_base);
		data->bd.cpar.iov_base = 0;
		data->bd.cpar.iov_len  = 0;
#endif
		break;
	  default:;
	}
}

extern void mpt_bacol_init(MPT_SOLVER_STRUCT(bacol) *data)
{
	MPT_IVPPAR_INIT(&data->ivp);
	
	MPT_VECPAR_INIT(&data->rtol, __MPT_IVP_RTOL);
	MPT_VECPAR_INIT(&data->atol, __MPT_IVP_ATOL);
	
	data->kcol   = 2;
	data->nintmx = 127;
	data->nint   = 10;
	
	(void) memset(&data->mflag, 0, sizeof(data->mflag));
	data->mflag.noinit = -1;
	
	data->x = data->y = 0;
	
	data->rpar.iov_base = 0; data->rpar.iov_len = 0;
	data->ipar.iov_base = 0; data->ipar.iov_len = 0;
	data->owrk.iov_base = 0; data->owrk.iov_len = 0;
	
	data->ufcn = 0;
	
	data->xinit = mpt_bacol_grid_init;
	data->xgrid = 0;
	
	*((short *) &data->backend) = 'd';
	
	memset(&data->bd, 0, sizeof(data->bd));
}

