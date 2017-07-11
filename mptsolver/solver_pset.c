/*!
 * set specific solver parameter from configuration list.
 */

#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>

#include "meta.h"
#include "node.h"
#include "object.h"

#include "client.h"

/*!
 * \ingroup mptSolver
 * \brief set solver properties
 * 
 * Set parameters from configuration elements.
 * 
 * \param gen  solver descriptor
 * \param conf configuration list
 * \param mask errors to ignore
 * \param out  logging descriptor
 */
extern void mpt_solver_pset(MPT_INTERFACE(object) *obj, const MPT_STRUCT(node) *conf, int mask, MPT_INTERFACE(logger) *out)
{
	MPT_STRUCT(property) pr;
	
	if (!conf) {
		return;
	}
	pr.name = "";
	pr.desc = 0;
	if (obj->_vptr->property(obj, &pr) < 0 || !pr.name) {
		pr.name = "solver";
	}
	do {
		const MPT_INTERFACE(metatype) *mt;
		const char *name, *str = 0;
		int flg, ret;
		
		flg = conf->children ? MPT_ENUM(TraverseNonLeafs) : MPT_ENUM(TraverseLeafs);
		
		if (mask & flg) {
			continue;
		}
		if (!mpt_identifier_len(&conf->ident)
		 || !(name = mpt_identifier_data(&conf->ident))) {
			/* avoid empty property */
			if ((mask & MPT_ENUM(TraverseEmpty))) {
				continue;
			}
			if (out) {
				mpt_log(out, __func__, MPT_LOG(Error), "%s: %s: <%p>",
				        pr.name, MPT_tr("no property name"), conf);
			}
			continue;
		}
		/* skip default value */
		if (!(mt = conf->_meta)) {
			if (!(mask & MPT_ENUM(TraverseDefault))) {
				if (out) {
					mpt_log(out, __func__, MPT_LOG(Error), "%s: %s: %s",
					        pr.name, MPT_tr("no element value"), name);
				}
				continue;
			}
			ret = obj->_vptr->setProperty(obj, name, 0);
		} else if ((str = mpt_meta_data(mt, 0))) {
			ret = mpt_object_set_string(obj, name, str, 0);
		} else {
			ret = obj->_vptr->setProperty(obj, name, mt);
		}
		if (ret >= 0) {
			if (out) {
				mpt_log(out, __func__, MPT_LOG(Debug), "%s: %s.%s",
				        MPT_tr("assigned property"), pr.name, name);
			}
			continue;
		}
		if (ret == MPT_ERROR(BadArgument)) {
			if ((mask & MPT_ENUM(TraverseUnknown))) {
				continue;
			}
			if (out) {
				mpt_log(out, __func__, MPT_LOG(Error), "%s: %s: %s",
				        pr.name, MPT_tr("bad property name"), name);
			}
			continue;
		}
		if (!out) {
			continue;
		}
		if (str) {
			mpt_log(out, __func__, MPT_LOG(Error), "%s: %s.%s = %s",
			        MPT_tr("bad property value"), pr.name, name, str);
			continue;
		}
		flg = mt->_vptr->conv(mt, 0, 0);
		if (isalnum(flg)) {
			mpt_log(out, __func__, MPT_LOG(Error), "%s: %s.%s = <%c>",
			        MPT_tr("bad property value"), pr.name, name, flg);
		} else {
			mpt_log(out, __func__, MPT_LOG(Error), "%s: %s.%s = <0x%x>",
			        MPT_tr("bad property value"), pr.name, name, flg);
		}
	} while ((conf = conf->next));
}

