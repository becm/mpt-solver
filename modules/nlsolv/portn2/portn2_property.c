/*!
 * set PORT N2 solver parameter
 */

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <float.h>
#include <errno.h>
#include <ctype.h>

#include "release.h"
#include "version.h"

#include "portn2.h"

static int setNls(MPT_SOLVER_STRUCT(portn2) *data, MPT_INTERFACE(source) *src)
{
	if (!src) {
		return mpt_nlspar_set(&data->nls, src);
	}
	else {
		MPT_SOLVER_STRUCT(nlspar) nls = data->nls;
		int ret;
		
		if ((ret =  mpt_nlspar_set(&nls, src)) < 0) {
			return ret;
		}
		mpt_portn2_fini(data);
		mpt_portn2_init(data);
		data->nls = nls;
		return ret;
	}
}

extern int mpt_portn2_property(MPT_SOLVER_STRUCT(portn2) *data, MPT_STRUCT(property) *prop, MPT_INTERFACE(source) *src)
{
	const char *name;
	intptr_t pos = -1, id;
	
	if (!prop) return (src && data) ? setNls(data, src) : MPT_ENUM(TypeSolver);
	
	if (!(name = prop->name)) {
		if (src) {
			errno = EINVAL;
			return MPT_ERROR(BadOperation);
		}
		if ((pos = (intptr_t) prop->desc) < 0) {
			errno = EINVAL;
			return MPT_ERROR(BadArgument);
		}
	}
	else if (!*name) {
		id = MPT_SOLVER_ENUM(NlsVector) | MPT_SOLVER_ENUM(NlsOverDet);
		if (data && src && (pos = setNls(data, src)) < 0) return pos;
		prop->name = "portn2"; prop->desc = "solver for overdetermined nonlinear equotations";
		prop->val.fmt = "ii"; prop->val.ptr = &data->nls;
		return id;
	}
	
	if (name && !strcasecmp(name, "version")) {
		static const char version[] = MPT_VERSION"\0";
		prop->name = "version"; prop->desc = "solver release information";
		prop->val.fmt = 0; prop->val.ptr = version;
		return 0;
	}
	
	errno = EINVAL;
	return MPT_ERROR(BadArgument);
}
