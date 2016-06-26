/*!
 * set specific solver parameter from configuration list.
 */

#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>

#include "node.h"

#include "object.h"

#include "client.h"

struct outputProc {
	MPT_INTERFACE(logger) *log;
	MPT_INTERFACE(object) *obj;
	const char *name;
	int err, mask;
};

static int procProp(void *ptr, const MPT_INTERFACE(property) *pr)
{
	static const char fcn[] = "mpt_solver_pset";
	struct outputProc *o = ptr;
	int ret;
	
	/* complain on missing name */
	if (!pr->name) {
		if (o->log) {
			(void) mpt_log(o->log, fcn, MPT_FCNLOG(Error), "%s: %s: <%p>",
			               o->name, MPT_tr("no property name"), pr->val.ptr);
		}
		o->err |= 8;
		return 4;
	}
	if ((ret = mpt_object_pset(o->obj, pr->name, &pr->val, 0)) >= 0) {
		return 0;
	}
	/* unknown property */
	if (ret == MPT_ERROR(BadArgument)) {
		o->err |= 1;
		if (!(o->mask & MPT_ENUM(TraverseUnknown)) || !o->log) {
			return 1;
		}
		(void) mpt_log(o->log, fcn, MPT_FCNLOG(Error), "%s: %s: %s",
		               o->name, MPT_tr("bad property name"), pr->name);
		return 1;
	}
	/* bad property value */
	if (ret == MPT_ERROR(BadValue)) {
		const char *tmp;
		o->err |= 2;
		if (!o->log) {
			return 2;
		}
		if ((tmp = pr->val.fmt)) {
			(void) mpt_log(o->log, fcn, MPT_FCNLOG(Error), "%s: %s: %s <%s>",
			               o->name, MPT_tr("bad property value"), pr->name, tmp);
		} else {
			if (!(tmp = pr->val.ptr)) tmp = "";
			(void) mpt_log(o->log, fcn, MPT_FCNLOG(Error), "%s: %s: %s = %s",
			               o->name, MPT_tr("bad property value"), pr->name, tmp);
		}
		return 2;
	}
	/* other property set error */
	(void) mpt_log(o->log, fcn, MPT_FCNLOG(Error), "%s: %s: %s",
	               o->name, MPT_tr("error setting property"), pr->name);
	o->err |= 4;
	return 3;
}

/*!
 * \ingroup mptSolver
 * \brief set solver property
 * 
 * Set parameters from configuration elements.
 * 
 * \param gen  solver descriptor
 * \param conf configuration list
 * \param mask errors to ignore
 * \param out  logging descriptor
 */
extern int mpt_solver_pset(MPT_INTERFACE(object) *obj, const MPT_STRUCT(node) *conf, int mask, MPT_INTERFACE(logger) *out)
{
	struct outputProc proc;
	MPT_STRUCT(property) pr;
	
	proc.log = out;
	proc.obj = obj;
	proc.mask = mask;
	proc.err = 0;
	
	pr.name = "";
	pr.desc = 0;
	if (obj->_vptr->property(obj, &pr) < 0 || !(proc.name = pr.name)) {
		proc.name = "solver";
	}
	
	return mpt_node_foreach(conf, procProp, &proc, mask) ? -1 : proc.err;
}

