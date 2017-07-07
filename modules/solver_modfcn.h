/*!
 * MPT solver module function definitions
 */

#include "../solver.h"

#ifndef MPT_SOLVER_MODULE_FCN
# define MPT_SOLVER_MODULE_FCN(x) _mpt_solver_module_##x
#endif
extern int MPT_SOLVER_MODULE_FCN(odefcn_set)(long , MPT_IVP_STRUCT(odefcn) *, int , const void *);
extern int MPT_SOLVER_MODULE_FCN(daefcn_set)(long , MPT_IVP_STRUCT(daefcn) *, int , const void *);
