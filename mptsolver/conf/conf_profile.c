/*!
 * set profile data from descriptions
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "node.h"
#include "meta.h"

#include "values.h"

/*!
 * \ingroup mptValues
 * \brief configure parameter data
 * 
 * Get profile data from configuration list.
 * Use ld<0 for alignment by profile.
 * 
 * \param len	length of profiles
 * \param dest	target address
 * \param ld	number of profiles
 * \param conf	profile configuration node
 * \param grid	grid data (needed for polynom type)
 * 
 * \return zero on success
 */
extern int mpt_conf_profile(int len, double *dest, int ld, const MPT_STRUCT(node) *conf, const double *grid)
{
	MPT_INTERFACE(metatype) *meta;
	const char *descr;
	int type;
	
	if (len <= 0 || !ld) {
		return MPT_ERROR(BadArgument);
	}
	if (!conf) {
		mpt_values_bound(len, dest, ld, 0, 0, 0);
	}
	if (!(descr = mpt_node_ident(conf))) {
		return MPT_ERROR(BadType);
	}
	if (!(meta = conf->_meta)) {
		return MPT_ERROR(BadValue);
	}
	if ((type = mpt_valtype_select(&descr)) < 0) {
		return type;
	}
	descr = mpt_meta_data(meta, 0);
	return mpt_valtype_init(type, descr, len, dest, ld, grid);
}

