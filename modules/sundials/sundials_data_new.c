/*!
 * Sundials N_Vector creation
 */

#include <string.h>

#include <nvector/nvector_serial.h>

#include "sundials.h"

#include "module_functions.h"

extern realtype *MPT_SOLVER_MODULE_FCN(data_new)(N_Vector *vec, long len, const realtype *from)
{
	MPT_SOLVER_MODULE_DATA_TYPE *dest;
	N_Vector nv, ov;
	size_t size;
	if (!(nv = mpt_sundials_nvector(len))) {
		return 0;
	}
	dest = N_VGetArrayPointer(nv);
	size = len * sizeof(*dest);
	if (from) {
		memcpy(dest, from, size);
	} else {
		memset(dest, 0, size);
	}
	if ((ov = *vec)) {
		N_VDestroy(ov);
	}
	*vec = nv;
	return dest;
}
