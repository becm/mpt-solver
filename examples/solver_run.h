/*!
 *  define NLS user init parameters
 */
#ifndef MPT_INCLUDE
# define MPT_INCLUDE(h) <mpt/h>
#endif
#include MPT_INCLUDE(core.h)
__MPT_NAMESPACE_BEGIN
MPT_INTERFACE(client);
__MPT_EXTDECL_BEGIN
extern int client_init(int , char * const []);
extern int solver_run(MPT_INTERFACE(client) *);
__MPT_EXTDECL_END
__MPT_NAMESPACE_END
