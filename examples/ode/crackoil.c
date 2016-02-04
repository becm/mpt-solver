/*!
 * ODE example: Crackoil
 */

#include <mpt/array.h>
#include <mpt/solver.h>

static double theta[3] = { 11.948, 7.993, 2.024 };

static int rh_side(void *udata, double t, const double *y, double *f)
{
	double qy0 = y[0] * y[0];
	
	(void) udata; (void) t;
	
	f[0] = -(theta[0] + theta[2]) * qy0;
	f[1] = theta[0] * qy0 - theta[1] * y[1];
	
	return 0;
}

extern int user_init(MPT_SOLVER_STRUCT(odefcn) *usr, MPT_SOLVER_STRUCT(data) *sd)
{
	double *param;
	int i, n;
	
	usr->fcn = rh_side;
	
	param = mpt_data_param(sd);
	
	n = sd->npar < 3 ? sd->npar : 3;
	
	for (i = 0; i < n; i++) {
		theta[i] = param[i];
	}
	return 2;
}

