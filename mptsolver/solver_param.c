/*!
 * MPT solver library
 *   setting solver parameters
 */

#include <string.h>
#include <ctype.h>

#include "meta.h"
#include "node.h"
#include "parse.h"
#include "output.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief set solver parameter
 * 
 * Set solver parameters from client configuration and argument string.
 * 
 * \param obj  solver descriptor
 * \param base client configuration list
 * \param out  logging descriptor
 */
extern int mpt_solver_param(MPT_INTERFACE(object) *obj, MPT_STRUCT(node) *base, MPT_INTERFACE(logger) *out)
{
	static const int maskSpecific = MPT_ENUM(TraverseNonLeafs) | MPT_ENUM(TraverseEmpty);
	static const int maskGeneric  = MPT_ENUM(TraverseNonLeafs) | MPT_ENUM(TraverseEmpty) | MPT_ENUM(TraverseUnknown) | MPT_ENUM(TraverseDefault);
	
	MPT_STRUCT(node) *conf, *sub;
	const char *fname;
	int ret;
	
	if ((conf = mpt_node_next(base, "solconf"))
	    && !(sub = conf->children)
	    && (fname = mpt_node_data(conf, 0))) {
		MPT_STRUCT(value) val;
		static const uint8_t fmt[] = "ss";
		const char *arg[2];
		
		val.fmt = fmt;
		val.ptr = arg;
		
		arg[0] = fname;
		arg[1] = "[ ] = !#";
		
		if ((ret = mpt_node_parse(conf, &val, out)) < 0) {
			mpt_log(out, __func__, MPT_LOG(Error), "%s",
			        MPT_tr("failed to read solver config"));
			return ret;
		}
		sub = conf->children;
	}
	if (!obj) {
		return 0;
	}
	if (sub) {
		MPT_STRUCT(property) pr;
		
		if (mpt_object_set_nodes(obj, ~maskGeneric, sub, out)) {
			ret |= 0x1;
		}
		pr.name = "";
		pr.desc = 0;
		
		if (obj->_vptr->property(obj, &pr) >= 0
		    && pr.name
		    && (conf = mpt_node_next(sub, pr.name))
		    && (sub = conf->children)
		    && mpt_object_set_nodes(obj, ~maskSpecific, sub, out)) {
			ret |= 0x2;
		}
		
	}
	if ((conf = mpt_node_next(base, "solver"))
	    && (sub = conf->children)
	    && mpt_object_set_nodes(obj, ~maskSpecific, sub, out)) {
		ret |= 0x10;
	}
	return ret;
}
