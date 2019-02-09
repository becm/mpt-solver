/*!
 * wrapper for IDA Jacobian
 */

#include <stdio.h>

#include <sunmatrix/sunmatrix_dense.h>
#include <sunmatrix/sunmatrix_band.h>

#include <ida/ida.h>

#include "sundials.h"

static int sundials_ida_jacobian(MPT_SOLVER_STRUCT(ida) *ida, long int n, double t, const double *y, double cj, double *jac, int ldjac)
{
	const MPT_IVP_STRUCT(daefcn) *dae = ida->ufcn;
	int ret;
	
	/* calculate jacobian */
	if ((ret = dae->jac.fcn(dae->jac.par, t, y, jac, ldjac)) < 0) {
		return ret;
	}
	/* Jac -= cj*B */
	if (!dae->mas.fcn) {
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
		
		if (!(mas = mpt_sundials_ida_tmp(ida, sizeof(*mas) + sizeof(*idrow) + sizeof(*idcol), max))) {
			return -1;
		}
		idrow = (int *)(mas + max);
		idcol = idrow + max;
		
		for (i = 0; i <= nint; i++) {
			int nz, j;
			
			*idrow = max;
			*idcol = i;
			
			if ((nz = dae->mas.fcn(dae->mas.par, t, y, mas, idrow, idcol)) < 0) {
				return nz;
			}
			for (j = 0 ; j < nz ; j++) {
				jac[(i * neqs + idcol[j]) * ldjac + idrow[j]] -= cj * mas[j];
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
extern int mpt_sundials_ida_jac(realtype t, realtype cj,
                                N_Vector y, N_Vector yp, N_Vector f,
                                SUNMatrix Jac, MPT_SOLVER_STRUCT(ida) *ida,
                                N_Vector tmp1, N_Vector tmp2, N_Vector tmp3)
{
	const MPT_IVP_STRUCT(daefcn) *fcn;
	double *jac;
	sunindextype ld, n;
	
	if (!ida || !(fcn = ida->ufcn) || !fcn->jac.fcn) {
		return IDA_MEM_NULL;
	}
	(void) yp; (void) f;
	(void) tmp1; (void) tmp2; (void) tmp3;
	
	ld = SUNMatGetID(Jac);
	
	if (ld == SUNMATRIX_DENSE) {
		jac = SM_DATA_D(Jac);
		ld  = SM_ROWS_D(Jac);
		n   = SM_LDATA_D(Jac);
	}
	else if (ld == SUNMATRIX_BAND) {
		jac = SM_DATA_B(Jac);
		ld  = SM_LDIM_B(Jac);
		n   = SM_LDATA_B(Jac);
	}
	else {
		return MPT_ERROR(BadArgument);
	}
	
	return sundials_ida_jacobian(ida, n, t, N_VGetArrayPointer(y), cj, jac, ld);
}
