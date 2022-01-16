/*!
 * MPT solver library
 *   setting solver parameters
 */

#include <string.h>
#include <ctype.h>
#include <stdio.h>

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
		static const char format[] = "[ ] = !#\0";
		FILE *file;
		
		if (!(file = fopen(fname, "r"))) {
			mpt_log(out, __func__, MPT_LOG(Error), "%s: %s",
			        MPT_tr("failed to open solver config"), fname);
			return MPT_ERROR(BadValue);
		}
		if ((ret = mpt_node_parse(conf, file, format, 0, out)) < 0) {
			mpt_log(out, __func__, MPT_LOG(Error), "%s",
			        MPT_tr("failed to read solver config"));
			return ret;
		}
		else {
			MPT_STRUCT(value) val = MPT_VALUE_INIT('s', &fname);
			mpt_meta_set(&conf->_meta, &val);
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
