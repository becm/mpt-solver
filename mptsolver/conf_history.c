/*!
 * MPT solver library
 *   read output parameter from configuration list
 */

#include "node.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief set history parameters
 * 
 * Use history parameters in node configuration list.
 * 
 * \param out   output config object
 * \param conf  configuration list
 * 
 * \return combined result of history configuration
 */
extern int mpt_conf_history(MPT_INTERFACE(object) *out, const MPT_STRUCT(node) *conf)
{
	static const int flags = MPT_ENUM(TraverseLeafs) | MPT_ENUM(TraverseChange) | MPT_ENUM(TraverseDefault);
	const char *data;
	int ret;
	
	if (!out) {
		return 0;
	}
	if (!conf) {
		out->_vptr->set_property(out, 0, 0);
		return 0;
	}
	/* process output config */
	data = mpt_node_data(conf, 0);
	conf = conf->children;
	if (!conf) {
		if ((ret = mpt_object_set_string(out, 0, data, 0)) < 0) {
			return ret;
		}
		return 1;
	}
	return mpt_object_set_nodes(out, flags, conf, 0);
}
