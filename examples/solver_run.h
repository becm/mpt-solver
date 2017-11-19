/*!
 * define solver runtime functions
 */
#ifndef MPT_INCLUDE
# define MPT_INCLUDE(h) <mpt/h>
#endif

#include MPT_INCLUDE(solver.h)

__MPT_NAMESPACE_BEGIN
MPT_INTERFACE(client);
__MPT_EXTDECL_BEGIN
extern int solver_run(MPT_INTERFACE(client) *);
__MPT_EXTDECL_END
__MPT_NAMESPACE_END
