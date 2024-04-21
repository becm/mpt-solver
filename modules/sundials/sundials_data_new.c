/*!
 * Sundials N_Vector creation
 */

#include <string.h>

#include <nvector/nvector_serial.h>

#include "sundials.h"

#include "module_functions.h"

extern sunrealtype *MPT_SOLVER_MODULE_FCN(data_new)(const MPT_SOLVER_STRUCT(sundials_vector_context) *vctx, long len, const sunrealtype *from)
{
	MPT_SOLVER_MODULE_DATA_TYPE *dest;
	N_Vector nv, ov;
	size_t size;
#if SUNDIALS_VERSION_MAJOR >= 6
	if (!(nv = mpt_sundials_nvector(len, vctx->ctx))) {
#else
	if (!(nv = mpt_sundials_nvector(len))) {
#endif
		return 0;
	}
	dest = N_VGetArrayPointer(nv);
	size = len * sizeof(*dest);
	if (from) {
		memcpy(dest, from, size);
	} else {
		memset(dest, 0, size);
	}
	if ((ov = *vctx->target)) {
		N_VDestroy(ov);
	}
	*vctx->target = nv;
	return dest;
}
