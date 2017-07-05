/*!
 * initialize/free PORT N2 instance
 */

#include "portn2.h"

extern void mpt_portn2_fini(MPT_SOLVER_STRUCT(portn2) *n2)
{
/* 	mpt_vecpar_alloc(&n2->pv, 0, 0);*/
	mpt_solver_valloc(&n2->rv, 0, 0);
	mpt_solver_valloc(&n2->iv, 0, 0);
}

extern int mpt_portn2_init(MPT_SOLVER_STRUCT(portn2) *n2)
{
	const MPT_SOLVER_NLS_STRUCT(parameters) par = MPT_NLSPAR_INIT;
	
	n2->nls = par;
	n2->pv.iov_base = 0;
	n2->pv.iov_len  = 0;
	
	n2->iv.iov_base = 0;
	if (!mpt_solver_valloc(&n2->iv, 82, sizeof(int))) {
		return -1;
	}
	n2->rv.iov_base = 0;
	if (!mpt_solver_valloc(&n2->rv, 105, sizeof(double))) {
		mpt_solver_valloc(&n2->iv, 0, 0);
		return -1;
	}
	
	((int *) n2->iv.iov_base)[0] = -1; /* invalid state */
	n2->nd = -1;  /* numeric jacobian by default */
	
	n2->res.res = n2->jac.jac = 0;
	
	n2->ui = 0;
	n2->ur = 0;
	
	n2->uf = 0;
	
	return 0;
}

