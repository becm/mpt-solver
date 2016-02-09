/*!
 *  create main routine for PDE problems
 */
#include <mpt/solver.h>
extern int user_init(MPT_SOLVER(IVP) *, MPT_SOLVER_STRUCT(data) *, MPT_INTERFACE(logger) *);
#define CREATE_CLIENT(x) mpt_client_ivp(user_init, (x))
#include "../main.c"
