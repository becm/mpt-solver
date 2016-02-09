/*!
 * create double vectors of specified length
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "solver.h"

extern int mpt_vecpar_settol(MPT_SOLVER_TYPE(dvecpar) *vec, MPT_INTERFACE(metatype) *src, double def)
{
	double *tol = vec->base;
	int ret;
	
	if ((ret = mpt_vecpar_set(&tol, 0, src)) < 0) {
		return ret;
	}
	if (ret < 2) {
		if (!ret) {
			vec->d.val = def;
		}
		if (tol) {
			if (ret) {
				vec->d.val = *tol;
			}
			free(tol);
		}
		vec->base = 0;
		return ret;
	}
	vec->base  = tol;
	vec->d.len = ret * sizeof(*tol);
	
	return ret;
}
