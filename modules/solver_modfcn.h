/*!
 * MPT solver module function definitions
 */

#include "../solver.h"

__MPT_SOLVER_BEGIN

#ifndef MPT_SOLVER_MODULE_DATA_TYPE
# define MPT_SOLVER_MODULE_DATA_TYPE double
# define MPT_SOLVER_MODULE_DATA_ID   'd'
#endif

#ifndef MPT_SOLVER_MODULE_DATA_CONTAINER
# define MPT_SOLVER_MODULE_DATA_CONTAINER MPT_SOLVER_MODULE_DATA_TYPE *
#endif

#ifndef MPT_SOLVER_MODULE_FCN
# error: solver module function name creator required
#endif

__MPT_EXTDECL_BEGIN

extern int MPT_SOLVER_MODULE_FCN(solver_conv)(const MPT_SOLVER(interface) *, const MPT_INTERFACE(object) *, MPT_TYPE(value) , void *);

extern int MPT_SOLVER_MODULE_FCN(ivp_vecset)(const MPT_IVP_STRUCT(parameters) *, MPT_SOLVER_MODULE_DATA_CONTAINER *, MPT_INTERFACE(convertable) *);
extern int MPT_SOLVER_MODULE_FCN(ivp_values)(const MPT_IVP_STRUCT(parameters) *, MPT_SOLVER_MODULE_DATA_TYPE , const MPT_SOLVER_MODULE_DATA_TYPE *, const char *, MPT_TYPE(property_handler) , void *);
extern int MPT_SOLVER_MODULE_FCN(ivp_state)(const MPT_IVP_STRUCT(parameters) *, double *, MPT_SOLVER_MODULE_DATA_CONTAINER *, MPT_INTERFACE(convertable) *);

extern MPT_SOLVER_MODULE_DATA_TYPE *MPT_SOLVER_MODULE_FCN(data_new)(MPT_SOLVER_MODULE_DATA_CONTAINER *, long , const MPT_SOLVER_MODULE_DATA_TYPE *);
extern int MPT_SOLVER_MODULE_FCN(data_set)(MPT_SOLVER_MODULE_DATA_TYPE *, int32_t , long , MPT_INTERFACE(iterator) *);

__MPT_EXTDECL_END

__MPT_SOLVER_END
