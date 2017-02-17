/*!
 *  define IVP user init parameters
 */

#include <mpt/array.h>  /* make solver data accessable */
#include <mpt/solver.h>
#include "solver_run.h"
extern int user_init(MPT_SOLVER(IVP) *, MPT_SOLVER_STRUCT(data) *, MPT_INTERFACE(logger) *);
