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
	size_t vis;
	
	mpt_path_set(&p, sub, -1);
	if (!(conf = mpt_config_node(&p))
	    || !(conf = conf->children)) {
		return 0;
	}
	/* compare first non-space block */
	vis = 0;
	while (*descr && isspace(*descr)) descr++;
	while (descr[vis] && !isspace(descr[vis])) vis++;
	
	do {
		MPT_INTERFACE(metatype) *mt;
		const char *id;
		
		if (!(id = mpt_identifier_data(&conf->ident))) {
			continue;
		}
		if (!(mt = conf->_meta)) {
			continue;
		}
		if (vis != strlen(id)) {
			continue;
		}
		if (strncmp(id, descr, vis)) {
			continue;
		}
		if (!(id = mpt_meta_data(mt, 0))) {
			continue;
		}
		return id;
	}
	while ((conf = conf->next));
	
	return 0;
}

