/*!
 * setup event controller for solver events
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <inttypes.h>

#include <string.h>
#include <sys/uio.h>

#include "event.h"
#include "client.h"

#include "config.h"
#include "meta.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief configure solver subtree
 * 
 * Set config files for userdata and solver parameters.
 * 
 * \param solv config interface to solver client
 * \param ev   event with config message
 * 
 * \return event error code on failure
 */
extern int mpt_solver_config(MPT_INTERFACE(object) *solv, MPT_INTERFACE(iterator) *it, MPT_INTERFACE(logger) *log)
{
	int pos;
	
	if (!it) {
		if ((pos = solv->_vptr->setProperty(solv, 0, 0)) < 0
		    && log) {
			mpt_log(log, __func__, MPT_LOG(Error), "%s",
			        MPT_tr("no default solver state"));
		}
		return pos;
	}
	pos = 0;
	while (1) {
		char buf[128];
		const char *cfg, *sep;
		size_t len;
		int ret;
		
		if ((ret = it->_vptr->get(it, 's', &cfg)) < 0) {
			if ((ret = it->_vptr->advance(it)) < 0) {
				if (log) {
					mpt_log(log, __func__, MPT_LOG(Error), "%s (%d)",
					        MPT_tr("bad argument type"), ret);
				}
				return pos ? pos : ret;
			}
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Warning), "%s (%d)",
				        MPT_tr("bad argument type"), ret);
			}
			continue;
		}
		if (!cfg) {
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Warning), "%s (%d)",
				        MPT_tr("no command content"), pos);
			}
			if ((ret = it->_vptr->advance(it)) <= 0) {
				return 0;
			}
			continue;
		}
		/* find property separator */
		if (!(sep = strchr(cfg, '='))) {
			if (pos) {
				if (log) {
					mpt_log(log, __func__, MPT_LOG(Error), "%s (%d): %s",
					        MPT_tr("no argument separator"), pos, cfg);
				}
				return pos ? pos : ret;
			}
			if ((ret = mpt_object_set_string(solv, 0, sep + 1, 0)) < 0) {
				if (log) {
					mpt_log(log, __func__, MPT_LOG(Error), "%s: %s",
					        MPT_tr("failed to read client config"), cfg);
				}
				return ret;
			}
			if ((ret = it->_vptr->advance(it)) <= 0) {
				return 0;
			}
			++pos;
			continue;
		}
		len = sep - cfg;
		/* deny initial setup */
		if (!len) {
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Error), "%s (%d)",
				        MPT_tr("base setup denied"), pos);
			}
			return pos ? pos : ret;
		}
		/* split property name */
		if (len >= sizeof(buf)) {
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Error), "%s (%d): %s",
				        MPT_tr("path too long"), pos, cfg);
			}
			return pos ? pos : ret;
		}
		cfg = memcpy(buf, cfg, len);
		buf[len] = 0;
		
		/* set and advance solver parameter */
		if ((ret = mpt_object_set_string(solv, cfg, sep + 1, 0)) < 0) {
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Error), "%s (%d): %s",
				        MPT_tr("failed to solver config"), pos, cfg);
			}
			return pos ? pos : ret;
		}
		if ((ret = it->_vptr->advance(it)) <= 0) {
			return 0;
		}
		++pos;
	}
}
