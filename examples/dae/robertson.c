/*!
 * DAE example: Robertson Problem
 */

#include "solver_run.h"

/* DY(i)/DY(j) */
int rh_side(void *udata, double t, const double *y, double *f)
{
	const double *param = udata;
	(void) t;
	
	/* ODEs */
	f[0] = -param[0] * y[0] + param[2] * y[1] * y[2];
	f[1] = param[0] * y[0] - param[2] * y[1] * y[2] - param[1] * y[1] * y[1];
	/* alg. */
	f[2] = y[0] + y[1] + y[2] - 1.0;
	
	return 0;
}

int bmat_bcf(void *udata, double t, const double *y, double *b, int *row_ind, int *col_ind)
{
	(void) udata; (void) t; (void) y;
	
	/* ODEs */
	row_ind[0] = 0; col_ind[0] = 0; b[0] = 1.0;
	row_ind[1] = 1; col_ind[1] = 1; b[1] = 1.0;
	
	return 2;
}

/* DF/DY */
int jac_eval(void *udata, double t, const double *y, double *jac, int ljac)
{
	const double *param = udata;
	(void) t; (void) y;
	
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
/* solver setup for DAE run */
static int robertson_init(MPT_INTERFACE(convertable) *sol, MPT_STRUCT(solver_data) *sd, MPT_INTERFACE(logger) *out)
{
	MPT_IVP_STRUCT(daefcn) usr = MPT_IVP_DAE_INIT;
	double *param;
	const int neqs = 3;
	int ret;
	
	if (sd->npar < 3) {
		return MPT_ERROR(MissingData);
	}
	param = mpt_solver_data_param(sd);
	
	usr.rside.fcn = rh_side;
	usr.jac.fcn   = jac_eval;
	usr.mas.fcn   = bmat_bcf;
	usr.rside.par = param;
	usr.jac.par   = param;
	
	if ((ret = mpt_init_dae(sol, &usr, neqs, out)) < 0) {
		return ret;
	}
	return neqs;
}
int main(int argc, char * const argv[])
{
	MPT_INTERFACE(client) *cl;
	if (mpt_init(argc, argv) < 0) {
		return 1;
	}
	cl = mpt_client_ivp(robertson_init);
	return solver_run(cl);
}
