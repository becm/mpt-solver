/*!
 * setup event controller for solver events
 */

#include <string.h>

#include "client.h"

#include "solver.h"

extern const char *mpt_solver_read(MPT_INTERFACE(client) *solv, MPT_INTERFACE(metatype) *src)
{
	const char *cl, *sol;
	int err;
	
	if (!src) {
		if (solv->_vptr->cfg.assign((void *) solv, 0, 0) < 0) {
			return MPT_tr("no default configuration");
		}
		return 0;
	}
	if ((err = src->_vptr->conv(src, 's' | MPT_ENUM(ValueConsume), &cl)) < 0) {
		return MPT_tr("unable to get client config filename");
	}
	if (!err) {
		if (solv->_vptr->cfg.assign((void *) solv, 0, 0) < 0) {
			return MPT_tr("no default configuration");
		}
		return 0;
	}
	if ((err = src->_vptr->conv(src, 's' | MPT_ENUM(ValueConsume), &sol)) < 0) {
		return MPT_tr("unable to get solver config filename");
	}
	if (!err) {
		sol = 0;
	}
	if (mpt_config_set((void *) solv, 0, cl, 0, 0) < 0) {
		return MPT_tr("bad client config filename");
	}
	if (sol && mpt_config_set((void *) solv, "solconf", cl, 0, 0) < 0) {
		return MPT_tr("bad solver config filename");
	}
	while (err) {
		MPT_STRUCT(property) pr;
		
		if ((err = src->_vptr->conv(src, MPT_ENUM(TypeProperty) | MPT_ENUM(ValueConsume), &pr)) < 0
		    || !pr.name) {
			return MPT_tr("required property for extended config");
		}
		if (err) {
			if (!strcmp(pr.name, "solconf")) {
				return MPT_tr("solver config is positional argument");
			}
			mpt_config_set((void *) solv, pr.name, cl, '.', 0);
		}
	}
	return 0;
}
