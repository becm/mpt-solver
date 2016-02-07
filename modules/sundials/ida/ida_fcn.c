/*!
 * wrapper for IDA right side
 */

#include <errno.h>

#include <ida/ida.h>

#include "sundials.h"

/*!
 * \ingroup mptSundialsIda
 * \brief IDA user function wrapper
 * 
 * Wrapper to call mpt::ivpfcn entries from IDA solver.
 * For parameter description see Sundials documatation.
 * 
 * \return result of user function
 */
extern int sundials_ida_fcn(realtype t, N_Vector y, N_Vector yp, N_Vector f, MPT_SOLVER_STRUCT(ida) *ida)
{
	const MPT_SOLVER_STRUCT(ivpfcn) *fcn;
	double *df, *dyp, *dy;
	long neqs;
	int ret, nint;
	
	if (!ida || !(fcn = ida->ufcn) || !fcn->dae.fcn) {
		errno = EFAULT;
		return IDA_MEM_NULL;
	}
	df  = N_VGetArrayPointer(f);
	dyp = N_VGetArrayPointer(yp);
	dy  = N_VGetArrayPointer(y);
	
	if ((ret = ((const MPT_SOLVER_STRUCT(pdefcn) *) fcn)->fcn(fcn->dae.param, t, dy, df, &ida->ivp, fcn->grid, fcn->rside)) < 0) {
		return ret;
	}
	neqs = ida->ivp.neqs;
	nint = ida->ivp.pint;
	
	if (!fcn->dae.mas) {
		long i;
		neqs *= nint + 1;
		/* f -= E*yp */
		for (i = 0; i < neqs; i++) {
			df[i] -= dyp[i];
		}
	}
	else {
		double *mas;
		int i, max, *idrow, *idcol;
		
		max = neqs * neqs;
		
		if (!(mas = sundials_ida_tmp(ida, sizeof(*mas) + sizeof(*idrow) + sizeof(*idcol), max))) {
			return -1;
		}
		idrow = (int *)(mas + max);
		idcol = idrow + max;
		
		for (i = 0; i <= nint; i++) {
			int nz, j;
			
			idrow[0] = max;
			idcol[0] = i;
			
			if ((nz = fcn->dae.mas(fcn->dae.param, t, dy, mas, idrow, idcol)) < 0) {
				return nz;
			}
			/* f -= B*yp */
			for (j = 0 ; j < nz; j++) {
				df[idrow[j]] -= mas[j] * dyp[idcol[j]];
			}
			dy += neqs;
			df += neqs;
		}
	}
	return ret;
}



