/*!
 * get temporal data for IDA solver
 */

#include <errno.h>
#include <stdlib.h>
#include <stdint.h>

#include <sundials/sundials_types.h>

#include "sundials.h"

/*!
 * \ingroup mptSundialsIda
 * \brief temporary IDA data
 * 
 * Prepare IDA temporary data for requested size
 * and return base address.
 * 
 * \param ida   IDA solver data
 * \param esze  element size
 * \param nelem number of needed elements
 * 
 * \return temporary data
 */
extern void *sundials_ida_tmp(MPT_SOLVER_STRUCT(ida) *ida, size_t esze, size_t nelem)
{
	void *tdata;
	
	if (ida->tmp.base && !ida->tmp.size) {
		return ida->tmp.base;
	}
	if (ida->tmp.size/esze >= nelem) {
		return ida->tmp.base;
	}
	if (SIZE_MAX/esze < nelem) {
		errno = EOVERFLOW;
		return 0;
	}
	esze *= nelem;
	
	if (!(tdata = realloc(ida->tmp.base, esze))) {
		return 0;
	}
	ida->tmp.size = esze;
	
	return ida->tmp.base = tdata;
}

