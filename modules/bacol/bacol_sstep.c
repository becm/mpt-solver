/*!
 * C wrapper to bacol fortran routines
 */

#include <errno.h>
#include <string.h>

#include "bacol.h"

extern int mpt_bacol_sstep(MPT_SOLVER_STRUCT(bacol) *data, double tend)
{
	MPT_SOLVER_STRUCT(ivppar) *ivp = &data->ivp;
	double *rtol, *atol;
	int idid, kcol, lip, lrp;
#ifdef MPT_BACOL_RADAU
	int lcp;
#endif
	if ( (idid = data->mflag.noinit) < 0 ) {
		errno = EINVAL; return -1;
	}
	if ( idid ) idid = 1;
	
	if ( data->rtol.base && data->atol.base && ivp->neqs > 1 ) {
		data->mflag.tvec = 1; rtol = data->rtol.base; atol = data->atol.base;
	}
	else {
		data->mflag.tvec = 0; rtol = &data->rtol.d.val; atol = &data->atol.d.val;
	}
	kcol = data->kcol;
	
	lrp = data->rpar.iov_len / sizeof(double);
	lip = data->ipar.iov_len / sizeof(int);
	
	/* fortran routine call */
	switch ( data->backend ) {
#ifdef MPT_BACOL_RADAU
	    case 'r': case 'R':
	lcp = data->bd.cpar.iov_len / sizeof(double) / 2;
	bacolr_(&ivp->last, &tend, atol, rtol, &ivp->neqs, &kcol,
		&data->nintmx, &data->nint, data->x, (int *) &data->mflag,
		data->rpar.iov_base, &lrp, data->ipar.iov_base, &lip,
		data->bd.cpar.iov_base, &lcp, data->y, &idid);
	    break;
#endif
#ifdef MPT_BACOL_DASSL
	    case 'd': case 'D':
	bacol_(&ivp->last, &tend, atol, rtol, &ivp->neqs, &kcol,
		&data->nintmx, &data->nint, data->x, (int *) &data->mflag,
		data->rpar.iov_base, &lrp, data->ipar.iov_base, &lip, data->y, &idid);
	    break;
#endif
	    default: errno = EBADR; return -1;
	}
	if ( idid < 0 ) errno = EINVAL;
	
	return idid;
}
