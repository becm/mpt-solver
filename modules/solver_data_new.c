/*!
 * set double values on vector
 */

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <sys/uio.h>

#include "meta.h"

#include "solver_modfcn.h"

extern MPT_SOLVER_MODULE_DATA_TYPE *MPT_SOLVER_MODULE_FCN(data_new)(MPT_SOLVER_MODULE_DATA_CONTAINER *y, long len, const MPT_SOLVER_MODULE_DATA_TYPE *from)
{
	MPT_SOLVER_MODULE_DATA_TYPE *dest;
	size_t size;
	size = len * sizeof(*dest);
	if (!(dest = malloc(size))) {
		return 0;
	}
	if (*y) {
		free(*y);
	}
	if (from) {
		*y = memcpy(dest, from, size);
	} else {
		*y = memset(dest, 0, size);
	}
	return dest;
}
