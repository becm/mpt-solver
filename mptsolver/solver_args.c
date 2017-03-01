/*!
 * set solver config files
 */

#include "config.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief set solver arguments
 * 
 * Use arguments as solver client and
 * backend configuration file names.
 * 
 * \param cfg   solver descriptor
 * \param args  command line arguments
 * 
 * \return number of consumed arguments
 */
extern int mpt_solver_args(MPT_INTERFACE(config) *cfg, char * const args[], int argc)
{
	const char *par;
	
	if (!argc-- || !(par = *args++)) {
		return 0;
	}
	if (mpt_config_set(cfg, 0, par, 0, 0) < 0) {
		return MPT_ERROR(BadArgument);
	}
	if (!argc-- || !(par = *args++)) {
		return 1;
	}
	if (mpt_config_set(cfg, "solconf", par, 0, 0) < 0) {
		return MPT_ERROR(BadArgument);
	}
	if (argc < 0 && (par = *args++)) {
		return MPT_ERROR(BadArgument);
	}
	return 2;
}
