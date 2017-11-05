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
extern int client_init(int , char * const []);
extern int solver_run(MPT_INTERFACE(client) *);
__MPT_EXTDECL_END
__MPT_NAMESPACE_END

#ifdef __cplusplus
extern int user_init(mpt::solver::interface *, mpt::solver_data *, mpt::logger *);
#else
extern int user_init(MPT_SOLVER(interface) *, MPT_STRUCT(solver_data) *, MPT_INTERFACE(logger) *);
#endif
