/*!
 * create main routine for PDE problems
 */
#include <mpt/solver.h>
extern int user_init(MPT_SOLVER(NLS) *, MPT_SOLVER_STRUCT(data) *, MPT_INTERFACE(logger) *log);
#define CREATE_CLIENT(x) mpt_client_nls(user_init, (x))
#include "../main.c"
