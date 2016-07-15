/*!
 * sys4.c: BACOL user functions for PDE system
 */

#include <mpt/solver.h>

void derivf_(
	double *t, double *x,
	double *u, double *ux, double *uxx,
	double *dfdu, double *dfdux, double *dfduxx,
	int *npde)
{
	double t1, t2, t4;
	
	(void) t; (void) x; (void) ux; (void) uxx; (void) npde;
	
	t1 = 1. - u[2] - u[3];
	t2 = t1 * t1;
	t4 = -30. * u[0] -1.5 -1000. * u[3] * t2 + 2000. * u[2] * u[3] * t1;
	dfdu[0] = -30. * t1;
	dfdu[4] = 0.;
	dfdu[8] = 1.5 + 3. * u[0];
	dfdu[12] = 30. * u[0];
	
	dfdu[1] = 0.;
	dfdu[5] = -30. * t1;
	dfdu[9] = 30. * u[1];
	dfdu[13] = 1.2 + 30. * u[1];
	
	dfdu[2] = 30. * t1;
	dfdu[6] = 0.;
	dfdu[10] = t4;
	dfdu[14] = t4;
	
	dfdu[3] = 0.;
	dfdu[7] = 30. * t1;
	dfdu[11] = t4;
	dfdu[15] = t4;
	
	dfdux[0] = -1.;
	dfdux[4] = 0.;
	dfdux[8] = 0.;
	dfdux[12] = 0.;
	
	dfdux[1] = 0.;
	dfdux[5] = -1.;
	dfdux[9] = 0.;
	dfdux[13] = 0.;
	
	dfdux[2] = 0.;
	dfdux[6] = 0.;
	dfdux[10] = 0.;
	dfdux[14] = 0.;
	
	dfdux[3] = 0.;
	dfdux[7] = 0.;
	dfdux[11] = 0.;
	dfdux[15] = 0.;
	
	dfduxx[0] = 0.01;
	dfduxx[4] = 0.;
	dfduxx[8] = 0.;
	dfduxx[12] = 0.;
	
	dfduxx[1] = 0.;
	dfduxx[5] = 0.01;
	dfduxx[9] = 0.;
	dfduxx[13] = 0.;
	
	dfduxx[2] = 0.;
	dfduxx[6] = 0.;
	dfduxx[10] = 0.01;
	dfduxx[14] = 0.;
	
	dfduxx[3] = 0.;
	dfduxx[7] = 0.;
	dfduxx[11] = 0.;
	dfduxx[15] = 0.01;
}

void difbxa_(
	double *t, double *u,
	double *ux, double *dbdu, double *dbdux,
	double *dbdt,
	int *npde)
{
	(void) t; (void) u; (void) ux; (void) npde;
	
	dbdu[0] = -100.;
	dbdu[4] = 0.;
	dbdu[8] = 0.;
	dbdu[12] = 0.;
	
	dbdu[1] = 0.;
	dbdu[5] = -100.;
	dbdu[9] = 0.;
	dbdu[13] = 0.;
	
	dbdu[2] = 0.;
	dbdu[6] = 0.;
	dbdu[10] = 0.;
	dbdu[14] = 0.;
	
	dbdu[3] = 0.;
	dbdu[7] = 0.;
	dbdu[11] = 0.;
	dbdu[15] = 0.;
	
	dbdux[0] = 1.;
	dbdux[4] = 0.;
	dbdux[8] = 0.;
	dbdux[12] = 0.;
	
	dbdux[1] = 0.;
	dbdux[5] = 1.;
	dbdux[9] = 0.;
	dbdux[13] = 0.;
	
	dbdux[2] = 0.;
	dbdux[6] = 0.;
	dbdux[10] = 1.;
	dbdux[14] = 0.;
	
	dbdux[3] = 0.;
	dbdux[7] = 0.;
	dbdux[11] = 0.;
	dbdux[15] = 1.;
	
	dbdt[0] = 0.;
	dbdt[1] = 0.;
	dbdt[2] = 0.;
	dbdt[3] = 0.;
	
	
}
void difbxb_(
	double *t, double *u,
	double *ux, double *dbdu, double *dbdux,
	double *dbdt,
	int *npde)
{
	(void) t; (void) u; (void) ux; (void) npde;
	
	dbdu[0] = 0.;
	dbdu[4] = 0.;
	dbdu[8] = 0.;
	dbdu[12] = 0.;
	
	dbdu[1] = 0.;
	dbdu[5] = 0.;
	dbdu[9] = 0.;
	dbdu[13] = 0.;
	
	dbdu[2] = 0.;
	dbdu[6] = 0.;
	dbdu[10] = 0.;
	dbdu[14] = 0.;
	
	dbdu[3] = 0.;
	dbdu[7] = 0.;
	dbdu[11] = 0.;
	dbdu[15] = 0.;
	
	dbdux[0] = 1.;
	dbdux[4] = 0.;
	dbdux[8] = 0.;
	dbdux[12] = 0.;
	
	dbdux[1] = 0.;
	dbdux[5] = 1.;
	dbdux[9] = 0.;
	dbdux[13] = 0.;
	
	dbdux[2] = 0.;
	dbdux[6] = 0.;
	dbdux[10] = 1.;
	dbdux[14] = 0.;
	
	dbdux[3] = 0.;
	dbdux[7] = 0.;
	dbdux[11] = 0.;
	dbdux[15] = 1.;
	
	dbdt[0] = 0.;
	dbdt[1] = 0.;
	dbdt[2] = 0.;
	dbdt[3] = 0.;
}
void bndxa_(double *t, double *u, double *ux, double *bval, int *npde)
{
	(void) t; (void) npde;
	
	bval[0] = ux[0] + 100. * (2. - .96 - u[0]);
	bval[1] = ux[1] + 100. * (.96 - u[1]);
	bval[2] = ux[2];
	bval[3] = ux[3];
}
void bndxb_(double *t, double *u, double *ux, double *bval, int *npde)
{
	(void) t; (void) u; (void) npde;
	
	bval[0] = ux[0];
	bval[1] = ux[1];
	bval[2] = ux[2];
	bval[3] = ux[3];
}
void uinit_(double *x, double *u, int *npde)
{
	(void) x; (void) u; (void) npde;
	
	u[0] = 1.04;
	u[1] = 0.96;
	u[2] = 0.0;
	u[3] = 0.0;
}

void f_(
	double *t, double *x,
	double *u, double *ux, double *uxx,
	double *fval,
	int *npde)
{
	double t1, t2;
	
	(void) t; (void) x; (void) npde;
	
	t1 = 1. - u[2] - u[3];
	t2 = t1 * t1;
	fval[0] = -ux[0] + 0.01 * uxx[0] + 1.5 * u[2] - 30. * u[0] * t1;
	fval[1] = -ux[1] + 0.01 * uxx[1] + 1.2 * u[3] - 30. * u[1] * t1;
	fval[2] = 0.01 * uxx[2] + 30. * u[0] * t1 - 1.5 * u[2] - 1000. * u[2] * u[3]* t2;
	fval[3] = 0.01 * uxx[3] + 30. * u[1] * t1 - 1.2 * u[3] - 1000. * u[2] * u[3]* t2;
}

/* map functions to bacol parameters */
extern int user_init(MPT_SOLVER(IVP) *sol, MPT_SOLVER_STRUCT(data) *sd, MPT_INTERFACE(logger) *log)
{
	int ret, npde = 4;
	
	(void) sd;
	if ((ret = mpt_object_set((void *) sol, "", "ii", npde, 0)) < 0) {
		mpt_log(log, __func__, MPT_LOG(Error), "%s", "unable to set PDE count");
		return ret;
	}
	/* no profile operation */
	return 0;
}
