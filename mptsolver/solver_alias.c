/*!
 * resolve alias to full library description
 */

#include <string.h>
#include <strings.h>
#include <ctype.h>

#include "node.h"
#include "config.h"

#include "client.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief resolve solver symbol
 * 
 * Resolve short name to dynamic symbol load string.
 * 
 * \param descr  solver short name
 * 
 * \return solver creator library description
 */
extern const char *mpt_solver_alias(const char *descr)
{
	static const char sub[] = "mpt.loader.alias\0";
	MPT_STRUCT(path) p = MPT_PATH_INIT;
	MPT_STRUCT(node) *conf;
	const char *id;
	int type, sol;
	
	mpt_path_set(&p, sub, -1);
	if (!(conf = mpt_config_node(&p))
	    || !(conf = conf->children)) {
		return 0;
	}
	if ((sol = mpt_solver_typeid()) < 0) {
		return 0;
	}
	if (!descr) {
		MPT_INTERFACE(metatype) *mt;
		
		if (!(mt = conf->_meta)) {
			return 0;
		}
		if (!(id = mpt_meta_data(mt, 0))) {
			return 0;
		}
		if ((type = mpt_proxy_typeid(id, 0)) != sol) {
			return 0;
		}
		return id;
	}
	/* compare non-space blocks */
	while (1) {
		MPT_STRUCT(node) *curr;
		size_t vis = 0;
		
		/* start/length of alias element */
		while (*descr && isspace(*descr)) descr++;
		while (descr[vis] && !isspace(descr[vis])) vis++;
		
		/* alias string terminated */
		if (!vis) {
			return 0;
		}
		/* get symbol for alias element */
		if ((curr = mpt_node_locate(conf, 1, descr, vis, -1))
		    && (id = mpt_node_data(curr, 0))
		    && (type = mpt_proxy_typeid(id, 0)) == sol) {
			return id;
		}
		/* advance alias alement */
		descr += vis;
	}
	return 0;
}

