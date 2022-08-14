/*!
 * MPT solver library
 *   add usage time difference
 */

#define _DEFAULT_SOURCE
#include <sys/time.h>
#include <sys/resource.h>

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief add time difference
 * 
 * Add system time difference collected before and after solver call.
 * 
 * \param t    sum ot sime differences
 * \param pre  usage collected before solver call
 * \param post usage collected after solver call
 */
extern void mpt_timeradd_sys(struct timeval *t, const struct rusage *pre, const struct rusage *post)
{
	struct timeval res;
	
	timersub(&post->ru_stime, &pre->ru_stime, &res);
	timeradd(t, &res, t);
}
/*!
 * \ingroup mptSolver
 * \brief add time difference
 * 
 * Add user time difference collected before and after solver call.
 * 
 * \param t    sum ot sime differences
 * \param pre  usage collected before solver call
 * \param post usage collected after solver call
 */
extern void mpt_timeradd_usr(struct timeval *t, const struct rusage *pre, const struct rusage *post)
{
	struct timeval res;
	
	timersub(&post->ru_utime, &pre->ru_utime, &res);
	timeradd(t, &res, t);
}
