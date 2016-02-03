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
	const MPT_SOLVER_STRUCT(daefcn) *dae;
	double *df, *dyp, *dy;
	long neqs;
	int ret, nint;
	
	if (!ida || !(dae = ida->ufcn) || !dae->fcn) {
		errno = EFAULT;
		return IDA_MEM_NULL;
	}
	df  = N_VGetArrayPointer(f);
	dyp = N_VGetArrayPointer(yp);
	dy  = N_VGetArrayPointer(y);
	
	if (ida->ivp.pint) {
		const MPT_SOLVER_STRUCT(pdefcn) *pde = (void *) dae;
		if ((ret = pde->fcn(pde->param, t, dy, df, &ida->ivp, pde->grid, pde->rside)) < 0) {
			return ret;
		}
	}
	else if ((ret = dae->fcn(dae->param, t, dy, df)) < 0) {
		return ret;
	}
	neqs = ida->ivp.neqs;
	nint = ida->ivp.pint;
	
	if (ida->ivp.pint || !dae->mas) {
		long i;
		neqs *= nint + 1;
		/* f -= E*yp */
		for (i = 0; i < neqs; i++) {
			df[i] -= dyp[i];
		}
	}
	else {
		double *mas;
		int i, *idrow, *idcol;
		
		i = neqs * neqs;
		
		if (!(mas = sundials_ida_tmp(ida, sizeof(*mas) + sizeof(*idrow) + sizeof(*idcol), i))) {
			return -1;
		}
		idrow = (int *)(mas + i);
		idcol = idrow + i;
		
		for (i = 0; i <= nint; i++) {
			int nz, j;
			
			if ((nz = dae->mas(dae->param, t, dy, mas, idrow, idcol)) < 0) {
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



