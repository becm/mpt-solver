/*!
 * define SUNDIALS solver module namespace
 */

#include <sundials/sundials_nvector.h>

#define MPT_SOLVER_MODULE_FCN(x) _mpt_sundials_##x
#include "sundials.h"

MPT_SOLVER_STRUCT(sundials_vector_context)
{
	N_Vector *target;
#if SUNDIALS_VERSION_MAJOR >= 6
	SUNContext ctx;
#endif
};

#define MPT_SOLVER_MODULE_DATA_TYPE      realtype
#define MPT_SOLVER_MODULE_DATA_ID        ((int) MPT_SOLVER_SUNDIALS(Realtype))
#define MPT_SOLVER_MODULE_DATA_CONTAINER const MPT_SOLVER_STRUCT(sundials_vector_context)

#include "solver_modfcn.h"
