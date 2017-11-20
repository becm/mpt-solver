/*!
 * setting solver parameters
 */

#include <string.h>
#include <ctype.h>

#include "node.h"
#include "meta.h"

#include "solver.h"

/*!
 * \ingroup mptClient
 * \brief set solver parameter
 * 
 * Set solver parameters from client configuration and argument string.
 * 
 * \param obj  solver descriptor
 * \param base client configuration list
 * \param arg  text type properties
 * \param out  logging descriptor
 */
extern void mpt_solver_param(MPT_INTERFACE(object) *obj, const MPT_STRUCT(node) *base, MPT_INTERFACE(logger) *out)
{
	static const int maskSpecific = MPT_ENUM(TraverseNonLeafs) | MPT_ENUM(TraverseEmpty);
	static const int maskGeneric  = MPT_ENUM(TraverseNonLeafs) | MPT_ENUM(TraverseEmpty) | MPT_ENUM(TraverseUnknown) | MPT_ENUM(TraverseDefault);
	
	MPT_STRUCT(node) *conf;
	MPT_STRUCT(property) pr;
	
	if ((conf = mpt_node_next(base, "solconf")) && (conf = conf->children)) {
		mpt_solver_pset(obj, conf, maskGeneric, out);
		
		pr.name = "";
		pr.desc = 0;
		
		if (obj->_vptr->property(obj, &pr) >= 0
		    && pr.name
		    && (conf = mpt_node_next(conf, pr.name))
		    && (conf = conf->children)) {
			mpt_solver_pset(obj, conf, maskSpecific, out);
		}
	}
	if ((conf = mpt_node_next(base, "solver")) && (conf = conf->children)) {
		mpt_solver_pset(obj, conf, maskGeneric, out);
	}
}
