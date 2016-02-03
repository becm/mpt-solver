/*!
 *  create main routine for DAE problems
 */
#include <mpt/solver.h>
extern int user_init(MPT_SOLVER_STRUCT(daefcn) *, MPT_SOLVER_STRUCT(data) *, MPT_INTERFACE(output) *);
#define CREATE_CLIENT(x) mpt_client_dae(user_init, (x))
#include "../main.c"
