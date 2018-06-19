/*!
 * wrapper to radau fortran routine.
 */

#include "radau.h"

extern int mpt_radau_step(MPT_SOLVER_STRUCT(radau) *rd, double tend)
{
	double *atol, *rtol;
	int neqs, tvec, imas, iout, ijac, idid, lrw, liw, *iwk;
	
	if (rd->count.st.nfev < 0 || !rd->fcn || !rd->y) {
		return MPT_ERROR(BadArgument);
	}
	neqs = rd->ivp.neqs * (rd->ivp.pint + 1);
	
	if (rd->ivp.neqs > 1 && (rtol = rd->rtol._base) && (atol = rd->atol._base)) {
		tvec = 1;
	} else {
		tvec = 0;
		rtol = &rd->rtol._d.val;
		atol = &rd->atol._d.val;
	}
	imas = rd->mas ? 1 : 0;
	iout = rd->sol ? 1 : 0;
	ijac = rd->jac && rd->ijac ? 1 : 0;
	
	liw = rd->iwork.iov_len / sizeof(int);
	lrw = rd->rwork.iov_len / sizeof(double);
	iwk = rd->iwork.iov_base;
	
	/* fortran routine call */
	radau_(&neqs, rd->fcn, &rd->t, rd->y, &tend, &rd->h, rtol, atol, &tvec,
	       rd->jac, &ijac, &rd->mljac, &rd->mujac,
	       rd->mas, &imas, &rd->mlmas, &rd->mumas,
	       rd->sol, &iout, rd->rwork.iov_base, &lrw, iwk, &liw,
	       rd->rpar, rd->ipar, &idid);
	
	for (tvec = 0; tvec < 6; tvec++) {
		rd->count.raw[tvec] += iwk[tvec+13];
	}
	return idid < 0 ? idid : 0;
}
