/*!
 * ODE example: Crackoil
 */

#include "solver_ivp.h"

static double theta[3] = { 11.948, 7.993, 2.024 };

static int rh_side(void *udata, double t, const double *y, double *f)
{
	double qy0 = y[0] * y[0];
	
	(void) udata; (void) t;
	
	f[0] = -(theta[0] + theta[2]) * qy0;
	f[1] = theta[0] * qy0 - theta[1] * y[1];
	
	return 0;
}

extern int user_init(MPT_SOLVER(generic) *sol, MPT_STRUCT(solver_data) *sd, MPT_INTERFACE(logger) *out)
{
	MPT_IVP_STRUCT(odefcn) usr = MPT_IVP_ODE_INIT;
	double *param;
	int i, n;
	
	usr.rside.fcn = rh_side;
	if ((i = mpt_init_ode(sol, &usr, 2, out)) < 0) {
		return i;
	}
	param = mpt_solver_data_param(sd);
	
	n = sd->npar < 3 ? sd->npar : 3;
	
	for (i = 0; i < n; i++) {
		theta[i] = param[i];
	}
	return 2;
}

