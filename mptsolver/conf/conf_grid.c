/*!
 * get grid start and dimension.
 */

#include <errno.h>

#include "array.h"
#include "convert.h"
#include "node.h"

#include "values.h"

/*!
 * \ingroup mptValues
 * \brief create grid data
 * 
 * Create grid values from configuration.
 * 
 * \param grid  target array
 * \param conf  configuration node for grid
 * 
 * \return size of grid
 */
extern int mpt_conf_grid(MPT_STRUCT(array) *grid, const MPT_STRUCT(node) *conf)
{
	static const char def_grid[] = "lin 0 1";
	const char *desc;
	int nint = 10, len;
	
	/* use default grid settings */
	if (!conf || !(desc = mpt_node_data(conf, 0))) {
		desc = def_grid;
	}
	/* get dimesion from grid description */
	else if ((len = mpt_cint(&nint, desc, 0, 0)) > 0) {
		if (nint < 1) return -2;
		desc += len;
	}
	if (grid->_buf) grid->_buf->used = 0;
	
	if (!mpt_values_generate(grid, ++nint, desc)) {
		return -1;
	}
	return nint;
}

