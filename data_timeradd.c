/*!
 * add usage time difference
 */

#define _BSD_SOURCE
#include <sys/time.h>
#include <sys/resource.h>

#include "array.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief add time difference
 * 
 * Add time difference collected before and after solver call.
 * 
 * \param md   client data descriptor
 * \param pre  usage collected before solver call
 * \param post usage collected after solver call
 */
extern void mpt_data_timeradd(MPT_SOLVER_STRUCT(data) *md, const struct rusage *pre, const struct rusage *post)
{
	struct timeval res;
	
	timersub(&post->ru_utime, &pre->ru_utime, &res);
	timeradd(&md->ru_usr, &res, &md->ru_usr);
	timersub(&post->ru_stime, &pre->ru_stime, &res);
	timeradd(&md->ru_sys, &res, &md->ru_sys);
}
