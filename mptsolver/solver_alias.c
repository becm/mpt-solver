/*!
 * MPT solver library
 *   resolve alias to full library description
 */

#include <string.h>
#include <strings.h>
#include <errno.h>

#include "config.h"
#include "client.h"

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
	const MPT_INTERFACE(metatype) *mt;
	const char *id;
	int type, sol;
	
	if ((sol = mpt_solver_typeid()) < 0) {
		errno = EINVAL;
		return 0;
	}
	if (!descr) {
		mt = mpt_config_get(0, base, '.', 0);
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
		
		mt = mpt_config_get(0, where, '.', 0);
	}
	if (!mt) {
		errno = ENOENT;
		return 0;
	}
	if (!(id = mpt_meta_data(mt, 0))) {
		return 0;
	}
	if ((type = mpt_alias_typeid(id, &id)) != sol) {
		errno = EINVAL;
		return 0;
	}
	return id;
}

