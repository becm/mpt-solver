/*!
 * DAE example: Robertson Problem
 */

#include <mpt/array.h>
#include <mpt/solver.h>

static double *param;
static int npar;

/* DY(i)/DY(j) */
int rh_side(void *udata, const double *t, const double *y, double *f)
{
	/* ODEs */
	f[0] = -param[0] * y[0] + param[2] * y[1] * y[2];
	f[1] = param[0] * y[0] - param[2] * y[1] * y[2] - param[1] * y[1] * y[1];
	/* alg. */
	f[2] = y[0] + y[1] + y[2] - 1.0;
	
	return 0;
}

int bmat_bcf(void *udata, const double *t, const double *y, double *b, int *row_ind, int *col_ind)
{
	/* ODEs */
	row_ind[0] = 0; col_ind[0] = 0; b[0] = 1.0;
	row_ind[1] = 1; col_ind[1] = 1; b[1] = 1.0;
	
	return 2;
}

/* DF/DY */
int jac_eval(void *udata, const double *t, const double *y, double *jac, int ljac)
{
	jac[0] = -param[0]; /* DF(i)/DY(1) */
	jac[1] =  param[0];
	jac[2] =  1.0;
	
	jac += ljac;
	
	jac[0] =  param[2] * y[2]; /* DF(i)/DY(2) */
	jac[1] = -param[2] * y[2] - 2 * param[1] * y[1];
	jac[2] =  1.0;
	
	jac += ljac;
	
	jac[0] =  param[2] * y[1]; /* DF(i)/DY(3) */
	jac[1] = -param[2] * y[1];
	jac[2] =  1.0;
	
	return 0;
}

int user_init(MPT_SOLVER_STRUCT(ivpfcn) *usr, MPT_SOLVER_STRUCT(data) *sd, MPT_INTERFACE(output) *out)
{
	usr->fcn = rh_side;
	usr->jac = jac_eval;
	usr->mas = bmat_bcf;
	
	param = mpt_data_param(sd);
	
	if ((npar = sd->npar) < 3) return -1;
	
	return 3;
}

