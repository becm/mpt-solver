/*!
 * MPT solver module function definitions
 */

#include "../solver.h"

#ifndef MPT_SOLVER_MODULE_DATA_CONTAINER
# define MPT_SOLVER_MODULE_DATA_CONTAINER double *
#endif

#ifndef MPT_SOLVER_MODULE_DATA_TYPE
# define MPT_SOLVER_MODULE_DATA_TYPE double
#endif

#ifndef MPT_SOLVER_MODULE_FCN
# define MPT_SOLVER_MODULE_FCN(x) _mpt_solver_module_##x
#endif

__MPT_SOLVER_BEGIN

__MPT_EXTDECL_BEGIN

extern int MPT_SOLVER_MODULE_FCN(odefcn_set)(long , MPT_IVP_STRUCT(odefcn) *, int , const void *);
extern int MPT_SOLVER_MODULE_FCN(daefcn_set)(long , MPT_IVP_STRUCT(daefcn) *, int , const void *);
extern int MPT_SOLVER_MODULE_FCN(ivpvec_set)(const MPT_IVP_STRUCT(parameters) *, MPT_SOLVER_MODULE_DATA_CONTAINER *, const MPT_INTERFACE(metatype) *);

__MPT_EXTDECL_END

__MPT_SOLVER_END
