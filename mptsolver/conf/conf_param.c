/*!
 * read solver data user parameter from configuration.
 */

#include <string.h>
#include <limits.h>
#include <float.h>
#include <errno.h>

#include "array.h"
#include "convert.h"
#include "node.h"

#include "values.h"

#include "solver.h"

/*!
 * \ingroup mptValues
 * \brief configure parameter data
 * 
 * Get parameter data from configuration
 * element or sublist.
 * Use parts<0 to auto-detect from (first) data element.
 * 
 * \param arr   target array descriptor
 * \param param parameter configuration node
 * \param parts parameters per line
 * 
 * \return parameter count
 */
extern int mpt_conf_param(MPT_STRUCT(array) *arr, const MPT_STRUCT(node) *param, int parts)
{
	const char *data;
	int npar = 0;
	
	if (!param) {
		return 0;
	}
	
	if (param->children) {
		const MPT_STRUCT(node) *tmp = param = param->children;
		double	*addr;
		int	max = 0;
		
		while (++max < INT_MAX && tmp->next) {
			tmp = tmp->next;
		}
		if (!(data = mpt_node_data(param, 0))) {
			parts = 1;
		}
		/* auto-detect parameter element count */
		else if (parts <= 0) {
			double	tmp;
			int len;
			parts = 0;
			
			while ((len = mpt_cdouble(&tmp, data, 0)) > 0) {
				data += len; parts++;
			}
			if (!parts) {
				parts = 1;
			}
		}
		if (!(addr = mpt_values_prepare(arr, parts * max)))
			return -1;
		
		do {
			int len;
			if (!(data = mpt_node_data(param, 0))) {
				continue;
			}
			if ((len = mpt_cdouble(addr, data, 0)) > 0) {
				int	i = 0;
				
				while (++i < parts) {
					if ((len = mpt_cdouble(addr+i*max, data+=len, 0)) <= 0) {
						break;
					}
				}
			}
			++addr;
			param = param->next;
		} while (++npar < max);
	}
	else {
		double	tmp;
		int len;
		
		if (!(data = mpt_node_data(param, 0)))
			return 0;
		
		/* add parameters to buffer */
		while ((len = mpt_cdouble(&tmp, data, 0)) > 0) {
			if (arr && !mpt_array_append(arr, sizeof(tmp), &tmp)) {
				break;
			}
			data += len;
			npar++;
		}
	}
	return npar;
}

