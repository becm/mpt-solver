/*!
 *  create main routine for DAE problems
 */
#include <mpt/solver.h>
extern int user_init(MPT_SOLVER_STRUCT(daefcn) *, const MPT_SOLVER_STRUCT(data) *);
#define CREATE_CLIENT(x) mpt_client_dae(user_init, (x))
#include "../main.c"
