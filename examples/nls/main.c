/*!
 * create main routine for PDE problems
 */
#include <mpt/solver.h>
extern int user_init(MPT_SOLVER_STRUCT(nlsfcn) *, MPT_SOLVER_STRUCT(data) *);
#define CREATE_CLIENT(x) mpt_client_nls(user_init, (x))
#include "../main.c"
