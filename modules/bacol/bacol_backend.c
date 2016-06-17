/*!
 * set BACOL backend
 */

#include <stdlib.h>
#include <ctype.h>

#include "bacol.h"

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
#ifdef MPT_BACOL_DASSL
	if (!val || !*val) val = "d";
	if (*val == 'd' || *val == 'D') {
		if (tolower(data->_backend) == tolower(*val)) {
			return 0;
		}
		if (data->bd.cpar.iov_base) {
			free(data->bd.cpar.iov_base);
		}
		data->mflag.noinit = 0;
		data->_backend = *val;
		return 1;
	}
#endif
#ifdef MPT_BACOL_RADAU
	if (!val || !*val) val = "r";
	if (*val == 'r' || *val == 'R') {
		if (tolower(data->_backend) == tolower(*val)) {
			return 0;
		}
		data->bd.tstop = 0.0;
		data->mflag.noinit = 0;
		data->_backend = *val;
		return 1;
	}
#endif
	return 1;
}
