/*!
 * resolve alias to full library description
 */

#include <string.h>
#include <strings.h>
#include <ctype.h>

#include "client.h"

static const struct
{
	const char *name;
	const char *sym;
}
assign[] = {
	{ "bacol",    "mpt_bacol_create@libmpt_bacol.so.1" },
	
	{ "vode",     "mpt_vode_create@libmpt_vode.so.1" },
	
	{ "dassl",    "mpt_dassl_create@libmpt_daesolv.so.1" },
	{ "radau",    "mpt_radau_create@libmpt_daesolv.so.1" },
	{ "mebdfi",   "mpt_mebdfi_create@libmpt_daesolv.so.1" },
	
	{ "limex",    "mpt_limex_create@libmpt_limex.so.1" },
	
	{ "cvode",    "sundials_cvode_create@libmpt_sundials.so.1" },
	{ "ida",      "sundials_ida_create@libmpt_sundials.so.1" },
	
	{ "minpack",  "mpt_minpack_create@libmpt_nlsolv.so.1" },
	{ "portn2",   "mpt_portn2_create@libmpt_nlsolv.so.1" }
};

/*!
 * \ingroup mptClient
 * \brief real solver creator
 * 
 * Resolve short name to dynamic symbol load string.
 * 
 * \param descr  solver short name
 * 
 * \return solver creator library description
 */
extern const char *mpt_solver_alias(const char *descr)
{
	size_t	i, vis;
	
	if (!descr)
		return 0;

	/* compare first non-space block */
	vis = 0;
	while (*descr && isspace(*descr)) descr++;
	while (descr[vis] && !isspace(descr[vis])) vis++;
	
	/* find library and initsymbol */
	i = 0;
	while (i < (int) (sizeof(assign)/sizeof(*assign))) {
		if (vis != strlen(assign[i].name) || strncasecmp(assign[i].name, descr, vis)) {
			i++; continue;
		}
		return assign[i].sym;
	}
	return 0;
}

