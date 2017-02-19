/*!
 *  DAE example: Chemical Akzo problem
 */

#include <stdio.h>
#include <math.h>

#include "solver_ivp.h"

static double
k1   = 18.7,
k2   = 0.58,
k3   = 0.09,
k4   = 0.42,
K    = 34.4,
klA  = 3.3,
Ks   = 115.83,
pco2 = 0.9,
H    = 737.0;

static int rs_akzo(void *udata, double t, const double *y, double *f)
{
	double Fin, r1, r2, r3, r4, r5, qy0, ry1;
	
	(void) udata; (void) t;
	
	if (y[1] < 0) {
		fprintf(stderr, "Fehler: y(2) ist kleiner als 0\n");
		return 1;
	}
	
	qy0 = y[0]*y[0];
	qy0 = qy0 * qy0;
	ry1 = sqrt(y[1]);
	
	r1 = k1  * qy0 * ry1;
	
	r2 = k2  *y[2]*y[3];
	r3 = k2/K*y[0]*y[4];
	r4 = k3  *y[0]*y[3]*y[3];
	
	r5 = k4  *y[5]*y[5]*ry1;
	Fin = klA*(pco2/H - y[1]);
	
	f[0] = -2.0*r1 + r2 - r3     - r4;
	f[1] = -0.5*r1               - r4 - 0.5*r5 + Fin;
	f[2] =      r1 - r2 + r3;
	f[3] =         - r2 + r3 - 2.0*r4;
	f[4] =           r2 - r3              + r5;
	
	f[5] = Ks*y[0]*y[3] - y[5];
	
	return 0;
}
static int bm_akzo(void *udata, double t, const double *y, double *b, int *idrow, int *idcol)
{
	int i, n = 5;
	
	(void) udata; (void) t; (void) y;
	
	for (i = 0; i < n; i++) {
		idrow[i] = i;
		idcol[i] = i;
		b[i] = 1.0;
	}
	return n;
}

int user_init(MPT_SOLVER(IVP) *sol, MPT_STRUCT(solver_data) *sd, MPT_INTERFACE(logger) *out)
{
	MPT_SOLVER_STRUCT(daefcn) *usr;
	double *param;
	
	if (!(usr = mpt_init_dae(sol, &sd->val, out))) {
		return MPT_ERROR(BadArgument);
	}
	usr->fcn = rs_akzo;
	usr->mas = bm_akzo;
	
	param = mpt_solver_data_param(sd);
	
	switch (sd->npar) {
	  default:;
	  case 9: H    = param[8];
	  case 8: pco2 = param[7];
	  case 7: Ks   = param[6];
	  case 6: klA  = param[5];
	  case 5: K    = param[4];
	  case 4: k4   = param[3];
	  case 3: k3   = param[2];
	  case 2: k2   = param[1];
	  case 1: k1   = param[0];
	  case 0:;
	}
	return 6;
}

