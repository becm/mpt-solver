/*!
 * define DAE solver module namespace
 */

#include <sundials/sundials_nvector.h>

#define MPT_SOLVER_MODULE_FCN(x) mpt_sundials_##x
#include "sundials.h"

#define MPT_SOLVER_MODULE_DATA_TYPE      realtype
#define MPT_SOLVER_MODULE_DATA_ID        ((int) MPT_SOLVER_ENUM(SundialsRealtype))
#define MPT_SOLVER_MODULE_DATA_CONTAINER N_Vector

#include "solver_modfcn.h"