/*!
 * C wrapper to MEBDFI fortran routine
 */

#include "mebdfi.h"

extern int mpt_mebdfi_step(MPT_SOLVER_STRUCT(mebdfi) *me, double tend)
{
	double *atol, *rtol, tout = tend;
	int itol, mf, ierr = 0, neqs, idid, lout, maxder, liw, lrw;
	
	if (me->state < 0 || !me->fcn || !me->y) {
		return MPT_ERROR(BadArgument);
	}
	neqs = me->ivp.neqs * (me->ivp.pint + 1);
	
	/* set tolerance flags and adresses */
	if (!(rtol = me->rtol.base)) {
		if ((atol = me->atol.base)) {
			itol = 3;
		} else {
			itol = 2; atol = &me->atol.d.val;
		}
		rtol = &me->rtol.d.val;
	}
	else if ((atol = me->atol.base)) {
		itol = 5;
	} else {
		itol = 4; atol = &me->atol.d.val;
	}
	if (!me->jac) {
		me->jnum = 1;
	}
	mf = me->jbnd ? (me->jnum ? 24 : 23) : (me->jnum ? 22 : 21);
	
	/* initial call */
	if ((idid = me->state) == 1 && !me->h) {
		me->h = (tend - me->t) / 128;
	}
	liw = me->iwork.iov_len / sizeof(int);
	lrw = me->rwork.iov_len / sizeof(double);
	lout = me->lout;
	maxder = me->maxder;
	
	/* fortran routine call */
	mebdfi_(&neqs, &me->t, &me->h, me->y, me->yp, &tout, &tend, &mf,
	        &idid, &lout, &lrw, me->rwork.iov_base, &liw, me->iwork.iov_base,
	        me->mbnd, &maxder, &itol, rtol, atol, me->rpar, me->ipar,
	        me->jac, me->fcn, &ierr);
	
	
	me->state = me->type;
	
	return ierr ? ierr : (idid < 0 ? idid : 0);
}
