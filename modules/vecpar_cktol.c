/*!
 * create double vectors of specified length
 */

#include <stdlib.h>
#include <string.h>

#include "../solver.h"

extern int mpt_vecpar_cktol(MPT_SOLVER_TYPE(dvecpar) *tol, int neqs, int post, double def)
{
	double *tmp, val;
	int  i, len;
	
	if (post < 0) {
		return MPT_ERROR(BadArgument);
	}
	if (neqs < 1) post = 0;
	
	if (!(tmp = tol->base)) {
		len = 0;
		val = tol->d.val;
		if (val <= 0.0) tol->d.val = val = def;
		if (!post) {
			return 0;
		}
	}
	else {
		len = tol->d.len / sizeof(*tmp);
		val = len ? tmp[0] : 0.0;
		if (val <= 0.0) val = def;
		if (!post) {
			free(tmp); tol->base = 0;
			tol->d.val = val;
			return 0;
		}
		if (len > neqs) len = neqs;
	}
	
	if (!(tmp = realloc(tmp, neqs * post * sizeof(*tmp)))) {
		return MPT_ERROR(BadOperation);
	}
	tol->d.len = neqs * post * sizeof(*tmp);
	
	for (i = 0; i < len; i++) {
		if (tmp[i] <= 0.0) tmp[i] = def;
	}
	for ( ; i < neqs ; i++) tmp[i] = val;
	
	len = (tmp == tol->base) ? 1 : 2;
	tol->base = tmp;
	
	/* copy values to additional positions */
	for (i = 1; i < post; i++) {
		tmp = memcpy(tmp+neqs, tmp, neqs*sizeof(*tmp));
	}
	return len;
}
