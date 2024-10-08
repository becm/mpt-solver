/*!
 * MPT solver library
 *   load missing solver client config files
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <unistd.h>

#include "output.h"
#include "config.h"

#include "loader.h"

#include "solver.h"

static char *stripFilename(char *base)
{
	size_t len;
	
	while (isspace(*base)) ++base;
	if (!(len = strlen(base))) {
		return 0;
	}
	while (len-- && isspace(base[len])) {
		base[len] = 0;
	}
	return base;
}

static int setFile(MPT_INTERFACE(config) *cfg, MPT_INTERFACE(logger) *info, const char *dest, const char *fname, const char *_func)
{
	if (access(fname, R_OK) < 0) {
		mpt_log(info, _func, MPT_LOG(Warning), "%s: %s",
		        MPT_tr("file not readable"), fname);
		return MPT_ERROR(BadValue);
	}
	if (mpt_config_set(cfg, dest, fname, 0, 0) < 0) {
		if (dest) {
			mpt_log(info, _func, MPT_LOG(Error), "%s: %s",
			        MPT_tr("failed to set solver config filename"), fname);
		    
		} else {
			mpt_log(info, _func, MPT_LOG(Error), "%s: %s",
			        MPT_tr("failed to set client config"), fname);
		}
		return MPT_ERROR(BadOperation);
	}
	return 0;
}

/*!
 * \ingroup mptSolver
 * \brief required solver config
 * 
 * Get missing solver client file names from user.
 * 
 * \param cfg   solver client config
 * \param info  log/error output target
 * 
 * \return event result or error
 */
extern int mpt_solver_require(MPT_INTERFACE(config) *cfg, MPT_INTERFACE(logger) *info)
{
	/* problem config filename from configuration/terminal */
	static const char defExt[] = "conf\0";
	const char *fname = 0, *cname = 0;
	char *rname, buf[128];
	int ret;
	
	/* check for existing config file */
	mpt_config_get(cfg, 0, 's', &fname);
	
	cname = 0;
	if ((mpt_config_get(0, "mpt", 's', &cname) > 0)
	 && cname) {
		const char *sep = strrchr(cname, '/');
		if (sep) {
			cname = sep + 1;
		}
	}
	if (!fname) {
		static const char defName[] = "client\0";
		const char *conf = cname ? cname : defName;
		snprintf(buf, sizeof(buf), "%s [%s.%s]: ",
		         MPT_tr("problem settings"), conf, defExt);
		
		if (!(rname = mpt_readline(buf))) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s (%s)",
			        MPT_tr("user interrupt"), MPT_tr("client config"));
			return MPT_ERROR(MissingData);
		}
		if (!(fname = stripFilename(rname))) {
			snprintf(buf, sizeof(buf), "%s.%s", conf, defExt);
			fname = buf;
		}
		ret = setFile(cfg, info, 0, fname, __func__);
		if (*rname) {
			free(rname);
		}
		if (ret < 0) {
			return ret;
		}
	}
	/* config needs solver settings */
	if (mpt_config_get(cfg, "solconf", 0, 0) < 0) {
		static const char defName[] = "solver\0", defPost[] = "sol\0";
		const char *sol = cname ? cname : defName;
		snprintf(buf, sizeof(buf), "%s [%s_%s.%s]: ",
		         MPT_tr("solver config"), sol, defPost, defExt);
		if (!(rname = mpt_readline(buf))) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s (%s)",
			        MPT_tr("user interrupt"), MPT_tr("solver config"));
			return MPT_ERROR(MissingData);
		}
		if (!(fname = stripFilename(rname))) {
			snprintf(buf, sizeof(buf), "%s_%s.%s", sol, defPost, defExt);
			fname = buf;
		}
		ret = setFile(cfg, info, "solconf", fname, __func__);
		if (*rname) {
			free(rname);
		}
		if (ret < 0) {
			return ret;
		}
	}
	return 0;
}

