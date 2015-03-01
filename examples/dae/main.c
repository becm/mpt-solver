/*!
 *  create main routine for DAE problems
 */
#include <mpt/solver.h>
extern int user_init(MPT_SOLVER_STRUCT(ivpfcn) *, MPT_SOLVER_STRUCT(data) *, MPT_INTERFACE(output) *);
#define CREATE_CLIENT() mpt_client_dae(user_init)
#include "../main.c"
