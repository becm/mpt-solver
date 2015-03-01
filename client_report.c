
#include <unistd.h>

#include <sys/time.h>

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief output client report
 * 
 * Print solver report and ellapsed time to output
 * 
 * \param gen  solver descriptor
 * \param out  output descriptor
 * \param usr  time spent in solver (userspace)
 * \param sys  time spent in solver (system)
 */
extern void mpt_client_report(MPT_INTERFACE(logger) *out, MPT_SOLVER_INTERFACE *gen, const struct timeval *usr, const struct timeval *sys)
{
	static const char *units[] = { "sec", "msec", "µsec" };
	int unit = 0;
	
	/* solver report output */
	mpt_log(out, 0, MPT_ENUM(LogMessage), " ");
	mpt_log(out, 0, MPT_ENUM(LogMessage), "%s", MPT_tr("solver report"));
	
	mpt_solver_report(gen, out);
	
	/* output time spent during solver call */
	mpt_log(out, 0, MPT_ENUM(LogMessage), " ");
	
	if (usr) {
		double t = usr->tv_usec;
		t = usr->tv_sec + t/1000000;
		if (t < 0.1) { ++unit; t *= 1000; }
		if (t < 0.1) { ++unit; t *= 1000; }
		mpt_log(out, 0, MPT_ENUM(LogMessage), "%-20s%18.4f [%s]", MPT_tr("user time:"), t, units[unit]);
	}
	if (sys) {
		double t = sys->tv_usec;
		t = sys->tv_sec + t/1000000;
		if (usr) {
			if (unit > 0) t *= 1000;
			if (unit > 1) t *= 1000;
		} else {
			if (t < 0.1) { ++unit; t *= 1000; }
			if (t < 0.1) { ++unit; t *= 1000; }
		}
		mpt_log(out, 0, MPT_ENUM(LogMessage), "%-20s%18.4f [%s]", MPT_tr("system time:"), t, units[unit]);
	}
}
