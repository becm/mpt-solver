/*!
 * resolve alias to full library description
 */

#include <string.h>
#include <strings.h>
#include <ctype.h>

#include "config.h"
#include "node.h"

#include "solver.h"

/*!
 * \ingroup mptClient
 * \brief real solver creator
 * 
 * Resolve short name to dynamic symbol load string.
 * 
 * \param descr  solver short name
 * 
 * \return solver creator library description
 */
extern const char *mpt_solver_alias(const char *descr)
{
	static const char sub[] = "mpt.solver.alias\0";
	MPT_STRUCT(path) p = MPT_PATH_INIT;
	MPT_STRUCT(node) *conf;
	
	mpt_path_set(&p, sub, -1);
	if (!(conf = mpt_config_node(&p))
	    || !(conf = conf->children)) {
		return 0;
	}
	if (!descr) {
		MPT_INTERFACE(metatype) *mt;
		
		if (!(mt = conf->_meta)) {
			return 0;
		}
		return mpt_meta_data(mt, 0);
	}
	/* compare non-space blocks */
	while (1) {
		MPT_STRUCT(node) *curr;
		MPT_STRUCT(metatype) *mt;
		const char *id;
		size_t vis = 0;
		
		/* start/length of alias element */
		while (*descr && isspace(*descr)) descr++;
		while (descr[vis] && !isspace(descr[vis])) vis++;
		
		/* alias string terminated */
		if (!vis) {
			return 0;
		}
		/* get symbol for alias element */
		if ((curr = mpt_node_locate(conf, 1, descr, vis))
		    && (mt = curr->_meta)
		    && (id = mpt_meta_data(mt, 0))) {
			return id;
		}
		/* advance alias alement */
		descr += vis;
	}
	return 0;
}

