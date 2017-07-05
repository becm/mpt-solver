/*!
 *  define NLS user init parameters
 */
#ifndef MPT_INCLUDE
# define MPT_INCLUDE(h) <mpt/h>
#endif
#include MPT_INCLUDE(array.h)  /* make solver data accessable */
#include MPT_INCLUDE(solver.h)
extern int user_init(MPT_SOLVER(generic) *, MPT_STRUCT(solver_data) *, MPT_INTERFACE(logger) *);
