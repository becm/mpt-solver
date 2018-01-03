/*!
 * medical AKZO problem
 */

#include "solver_run.h"

static const int N_PDE = 2;

/* solver right side calculation */
static int rs_pde(void *udata, double t, const double *y, double *f, const MPT_IVP_STRUCT(parameters) *ivp)
{
	const double *yr;
	double *fr, phi, zeta, dzeta, dzeta2, dum, alpha, beta, c = 4.0, k = 100.0;
	int npde, nint, i;
	
	(void) udata;
	
	npde = ivp->neqs;
	nint = ivp->pint;
	
	/* right boundary memory */
	yr = y + npde * nint;
	fr = f + npde * nint;
	
	phi = (t <= 5.0) ? 2.0 : 0;
	
	dzeta  = 1.0/(nint+1);
	dzeta2 = dzeta*dzeta;
	dum    = (dzeta-1.0)*(dzeta-1.0)/c;
	alpha  = 2.0*(dzeta-1.0)*dum/c;
	beta   = dum*dum;
	
/*        f(1) = (phi-2d0*y(1)+y(3))*beta/dzeta2
     +        +alpha*(y(3)-phi)/(2d0*dzeta)-k*y(1)*y(2) */
	f[0] = (phi-2.0*y[0]+y[2])*beta/dzeta2+alpha*(y[2]-phi)/(2.0*dzeta)-k*y[0]*y[1];
	/* f(2) = -k*y(1)*y(2) */
	f[1] = -k*y[0]*y[1];
	
	for (i = 1; i < nint; i++) {
		f += N_PDE;
		y += N_PDE;
		
		zeta  = (i+1)*dzeta;
		dum   = (zeta-1.0)*(zeta-1.0)/c;
		alpha = 2.0*(zeta-1.0)*dum/c;
		beta  = dum*dum;
		
	/*        f(i) = (y(i-2)-2d0*y(i)+y(i+2))*beta/dzeta2
	     +        +alpha*(y(i+2)-y(i-2))/(2d0*dzeta)-k*y(i)*y(i+1) */
		f[0] = (y[-2]-2.0*y[0]+y[2])*beta/dzeta2
		     + alpha*(y[2]-y[-2])/(2.0*dzeta)-k*y[0]*y[1];
		
		f[1] = -k*y[0]*y[1];
	}
	
	/* right boundary values */
	fr[0] = fr[1] = -k*yr[0]*yr[1];
	
	return 0;
}
/* solver/client setup for PDE run */
static int makzo_init(const MPT_INTERFACE(metatype) *sol, MPT_STRUCT(solver_data) *sd, MPT_INTERFACE(logger) *out)
{
	MPT_IVP_STRUCT(pdefcn) usr = MPT_IVP_PDE_INIT;
	int ret;
	usr.fcn = rs_pde;
	usr.par = mpt_solver_data_param(sd);
	if ((ret = mpt_init_pde(sol, &usr, N_PDE, &sd->val, out)) < 0) {
		return ret;
	}
	return N_PDE;
}
int main(int argc, char * const argv[])
{
	MPT_INTERFACE(client) *cl;
	if (mpt_init(argc, argv) < 0) {
		return 1;
	}
	cl = mpt_client_ivp(makzo_init);
	return solver_run(cl);
}
