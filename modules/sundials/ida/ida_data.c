/*!
 * handle opaque CVode data.
 */

#include <stdlib.h>

#include <sundials/sundials_types.h>

#include "sundials.h"

/*!
 * \ingroup mptSundialsIda
 * \brief IDA data initialization
 * 
 * Create and initialize IDA solver data.
 * 
 * \param fcn  ODE function pointer
 * 
 * \return initialized IDA data
 */
extern MPT_SOLVER_STRUCT(ida) *_mpt_sundials_ida_create(const MPT_IVP_STRUCT(daefcn) *fcn)
{
	MPT_SOLVER_STRUCT(ida) *ida;
	
	if (!(ida = malloc(sizeof(*ida)))) {
		return 0;
	}
	mpt_sundials_ida_init(ida);
	ida->ufcn = fcn;
	return ida;
}

/*!
 * \ingroup mptSundialsIda
 * \brief IDA data destruction
 * 
 * Destroy allocated IDA solver data.
 * 
 * \param ida  IDA solver data pointer
 */
extern const MPT_IVP_STRUCT(parameters) *_mpt_sundials_ida_parameters(const MPT_SOLVER_STRUCT(ida) *ida)
{
	return &ida->ivp;
}

/*!
 * \ingroup mptSundialsIda
 * \brief IDA data destruction
 * 
 * Destroy allocated IDA solver data.
 * 
 * \param ida  IDA solver data pointer created by _mpt_sundials_cvode_create()
 */
extern void _mpt_sundials_ida_destroy(MPT_SOLVER_STRUCT(ida) *ida)
{
	if (ida) {
		mpt_sundials_ida_fini(ida);
		free(ida);
	}
}
