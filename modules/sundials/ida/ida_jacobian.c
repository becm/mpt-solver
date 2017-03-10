/*!
 * wrapper for IDA Jacobian
 */

#include <sundials/sundials_direct.h>
#include <sundials/sundials_nvector.h>

#include "sundials.h"

static int sundials_ida_jacobian(MPT_SOLVER_STRUCT(ida) *ida, long int n, double t, const double *y, double cj, double *jac, int ldjac)
{
	const MPT_SOLVER_IVP_STRUCT(functions) *fcn = ida->ufcn;
	int ret;
	
	/* calculate jacobian */
	if ((ret = fcn->dae.jac(fcn->dae.param, t, y, jac, ldjac)) < 0) {
		return ret;
	}
	/* Jac -= cj*B */
	if (!fcn->dae.mas) {
		int i;
		for (i = 0; i < n; i++, jac += ldjac) {
			jac[i] -= cj;
		}
	}
	else {
		double *mas;
		int i, max, neqs, nint, *idrow, *idcol;
		
		neqs = ida->ivp.neqs;
		nint = ida->ivp.pint;
		max  = neqs * neqs;
		
		if (!(mas = sundials_ida_tmp(ida, sizeof(*mas) + sizeof(*idrow) + sizeof(*idcol), max))) {
			return -1;
		}
		idrow = (int *)(mas + max);
		idcol = idrow + max;
		
		for (i = 0; i <= nint; i++) {
			int nz, j;
			
			*idrow = max;
			*idcol = i;
			
			if ((nz = fcn->dae.mas(fcn->dae.param, t, y, mas, idrow, idcol)) < 0) {
				return nz;
			}
			for (j = 0 ; j < nz ; j++) {
				jac[(i*neqs+idcol[j])*ldjac+idrow[j]] -= cj * mas[j];
			}
			y += neqs;
		}
	}
	return ret;
}

/*!
 * \ingroup mptSundialsIda
 * \brief IDA banded jacobian wrapper
 * 
 * Wrapper to call mpt::ivpfcn jacobian from IDA solver.
 * For parameter description see Sundials documatation.
 * 
 * \return result of user jacobian function
 */
extern int sundials_ida_jac_band(long int n, long int mu, long int ml,
                                 realtype t, realtype cj,
                                 N_Vector y, N_Vector yp, N_Vector f,
                                 DlsMat Jac, MPT_SOLVER_STRUCT(ida) *ida,
                                 N_Vector tmp1, N_Vector tmp2, N_Vector tmp3)
{
	double *jac;
	int ld;
	
	(void) mu; (void) ml;
	(void) yp; (void) f;
	(void) tmp1; (void) tmp2; (void) tmp3;
	
	/* BAND_COL(Jac,i) is diagonal element */
	jac = BAND_COL(Jac,0);
	ld  = BAND_COL(Jac,1) - jac - 1;
	
	return sundials_ida_jacobian(ida, n, t, N_VGetArrayPointer(y), cj, jac, ld);
}

/*!
 * \ingroup mptSundialsIda
 * \brief IDA dense jacobian wrapper
 * 
 * Wrapper to call mpt::ivpfcn jacobian from IDA solver.
 * For parameter description see Sundials documatation.
 * 
 * \return result of user jacobian function
 */
extern int sundials_ida_jac_dense(long int n, realtype t, realtype cj,
                                  N_Vector y, N_Vector yp, N_Vector f,
                                  DlsMat Jac, MPT_SOLVER_STRUCT(ida) *ida,
                                  N_Vector tmp1, N_Vector tmp2, N_Vector tmp3)
{
	double *jac;
	int ld;
	
	(void) yp; (void) f;
	(void) tmp1; (void) tmp2; (void) tmp3;
	
	jac = DENSE_COL(Jac,0);
	ld  = DENSE_COL(Jac,1) - jac;
	
	return sundials_ida_jacobian(ida, n, t, N_VGetArrayPointer(y), cj, jac, ld);
}


