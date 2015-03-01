/*!
 * generic user functions LIMEX solver instance
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "limex.h"

struct _lxdata {
	int neqs, pint;
	MPT_SOLVER_STRUCT(ivpfcn) *ufcn;
};

static void limex_fcn(int *neq, int *nz, double *t, double *y, double *f, double *b, int *ir, int *ic, int *info)
{
	MPT_SOLVER_STRUCT(ivpfcn) *ufcn = ((struct _lxdata *) neq)->ufcn;
	int i, n;
	
	/* partial mass matrices */
	if (ufcn->mas) {
		const double *yt;
		int nint = ((struct _lxdata *) neq)->pint;
		int nt, neqs = *neq / (nint+1);
		
		for (i = 0, nt = 0, yt = y; i <= nint; i++) {
			int j;
			
			*ir = i;
			
			if ((n = ufcn->mas(ufcn->param, t, yt, b, ir, ic)) < 0) {
				*info = n;
				return;
			}
			for (j = 0; j < n; j++) {
				ic[j] += i*neqs + 1; ir[j] += 1;
			}
			nt  += n;
			b   += n;
			ir  += n;
			ic  += n;
			yt  += neqs;
		}
		*nz = nt;
	}
	/* identity matrix */
	else for (i = 0, n = *nz = *neq; i < n; i++) {
		ir[i] = ic[i] = i+1; b[i] = 1.;
	}
	if (!(n = ufcn->fcn(ufcn->param, t, y, f))) {
		*info = 0;
		return;
	}
	*info = (n < 0) ? -2 : -1;
}

static void limex_jac(int *neq, double *t, double *y, double *ys, double *jac, int *ldjac, int *ml, int *mu, int *banded, int *info)
{
	MPT_SOLVER_STRUCT(ivpfcn) *ufcn = ((struct _lxdata *) neq)->ufcn;
	int ld;
	
	(void) ys;
	(void) ml;
	
	if (!*banded)
		ld = *ldjac;
	else {
		/* Jac(i+k,j), k = mu + 1 - j */
		jac += *mu;
		ld   = *ldjac - 1;
	}
	*info = ufcn->jac(ufcn->param, t, y, jac, ld);
}

extern int mpt_limex_ufcn(MPT_SOLVER_STRUCT(limex) *data, const MPT_SOLVER_STRUCT(ivpfcn) *ufcn)
{
	if (!ufcn || !ufcn->fcn) {
		errno = EINVAL;
		return -1;
	}
	data->jac  = ufcn->jac ? limex_jac : 0;
	data->fcn  = limex_fcn;
	data->ufcn = ufcn;
	
	return 0;
}

