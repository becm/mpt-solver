/*!
 * set BACOL backend
 */

#include <stdlib.h>
#include <strings.h>
#include <ctype.h>

#include "bacol.h"

#ifdef MPT_BACOL_RADAU
void clearRadau(MPT_SOLVER_STRUCT(bacol) *data)
{
	if (!data->bd.cpar.iov_base) {
		return;
	}
	free(data->bd.cpar.iov_base);
	data->bd.cpar.iov_base = 0;
	data->bd.cpar.iov_len = 0;
}
#endif



/*!
 * \ingroup mptBacol
 * \brief set BACOL backend
 * 
 * Change backend of BACOL solver
 * 
 * \param bac  BACOL data
 * \param val  BACOL backend name
 * 
 * \retval <0  failure
 * \retval >=0 consumed value count
 */
extern int mpt_bacol_backend(MPT_SOLVER_STRUCT(bacol) *data, const char *val)
{
	void (*clear)(MPT_SOLVER_STRUCT(bacol) *) = 0;
	
	/* set fallback and destructor */
#ifdef MPT_BACOL_DASSL
	/* no special destruct operations */
#endif
#ifdef MPT_BACOL_RADAU
	/* need to free complex data array */
	if (data->_backend == 'r' || data->_backend == 'R') {
		clear = clearRadau;
	}
#endif
	/* setup for new backend */
#ifdef MPT_BACOL_DASSL
	if (!val || !strcasecmp(val, "dassl")) {
		if (data->_backend == 'd') {
			return 0;
		}
		if (clear) clear(data);
		data->_backend = 'd';
		data->bd.tstop = 0.0;
		return 1;
	}
#endif
#ifdef MPT_BACOL_RADAU
	if (!val || !strcasecmp(val, "radau")) {
		if (data->_backend == 'r') {
			return 0;
		}
		if (clear) clear(data);
		data->_backend = 'r';
		data->bd.cpar.iov_base = 0;
		data->bd.cpar.iov_len = 0;
		return 1;
	}
#endif
	return MPT_ERROR(BadValue);
}
