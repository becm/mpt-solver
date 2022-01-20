/*!
 * instantiate module helper functions
 * for SUNDIALS solver wrapper collection
 */

#include <string.h>

#include <sundials/sundials_nvector.h>
#include <sundials/sundials_matrix.h>
#include <sundials/sundials_linearsolver.h>

#include "sundials.h"

/*!
 * \ingroup mptSundials
 * \brief init SUNDIALS data
 * 
 * Initialize SUNDIALS data on raw memory.
 */
extern void mpt_sundials_init(MPT_SOLVER_STRUCT(sundials) *sd)
{
	memset(sd, 0, sizeof(*sd));
	sd->ml = sd->mu = -1;
}

#if SUNDIALS_VERSION_MAJOR >= 6
/*!
 * \ingroup mptSundials
 * \brief init SUNDIALS data
 * 
 * Initialize SUNDIALS data on raw memory.
 */
extern SUNContext mpt_sundials_context(MPT_SOLVER_STRUCT(sundials) *sd)
{
	if (!sd->_sun_ctx) {
		SUNContext_Create(sd->_sun_ctx_ref, &sd->_sun_ctx);
	}
	return sd->_sun_ctx;
}
#endif

/*!
 * \ingroup mptSundials
 * \brief finalize SUNDIALS data
 * 
 * Clear allocations on SUNDIALS data.
 */
extern void mpt_sundials_fini(MPT_SOLVER_STRUCT(sundials) *sd)
{
	if (sd->y) {
		N_VDestroy(sd->y);
	}
	if (sd->LS) {
		SUNLinSolFree(sd->LS);
	}
	if (sd->A) {
		SUNMatDestroy(sd->A);
	}
#if SUNDIALS_VERSION_MAJOR >= 6
	SUNContext_Free(&sd->_sun_ctx);
#endif
	memset(sd, 0, sizeof(*sd));
}
