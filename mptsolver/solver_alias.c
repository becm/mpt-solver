/*!
 * MPT solver library
 *   resolve alias to full library description
 */

#include <string.h>
#include <strings.h>
#include <errno.h>

#include "config.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief resolve solver symbol
 * 
 * Resolve short name to dynamic symbol load string.
 * 
 * \param descr  solver short name
 * 
 * \return solver creator library description
 */
extern const char *mpt_solver_alias(const char *descr)
{
	static const char base[] = "mpt.loader.alias\0";
	MPT_INTERFACE(convertable) *val;
	const MPT_STRUCT(named_traits) *sol;
	const char *id;
	
	if (!(sol = mpt_solver_type_traits())) {
		return 0;
	}
	if (!descr) {
		val = mpt_config_get(0, base, '.', 0);
	} else {
		char where[128];
		size_t blen, dlen;
		
		blen = strlen(base);
		dlen = strlen(descr);
		
		if ((blen + dlen + 2) > sizeof(where)) {
			errno = ENAMETOOLONG;
			return 0;
		}
		memcpy(where, base, blen);
		where[blen] = '.';
		memcpy(where + blen + 1, descr, dlen + 1);
		
		val = mpt_config_get(0, where, '.', 0);
	}
	if (!val) {
		errno = ENOENT;
		return 0;
	}
	if (!(id = mpt_convertable_data(val, 0))) {
		return 0;
	}
	if (mpt_alias_typeid(id, &id) != (int) sol->type) {
		errno = EINVAL;
		return 0;
	}
	return id;
}

