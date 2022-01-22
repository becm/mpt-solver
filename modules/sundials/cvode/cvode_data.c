/*!
 * handle opaque CVode data.
 */

#include <stdlib.h>

#include <sundials/sundials_types.h>

#include "sundials.h"

/*!
 * \ingroup mptSundialsCVode
 * \brief CVode data initialization
 * 
 * Create and initialize CVode solver data.
 * 
 * \param fcn  ODE function pointer
 * 
 * \return initialized CVode data
 */
extern MPT_SOLVER_STRUCT(cvode) *_mpt_sundials_cvode_create(const MPT_IVP_STRUCT(odefcn) *fcn)
{
	MPT_SOLVER_STRUCT(cvode) *cv;
	
	if (!(cv = malloc(sizeof(*cv)))) {
		return 0;
	}
	mpt_sundials_cvode_init(cv);
	cv->ufcn = fcn;
	return cv;
}

/*!
 * \ingroup mptSundialsCVode
 * \brief CVode data destruction
 * 
 * Destroy allocated CVode solver data.
 * 
 * \param cv  CVode solver data pointer
 * 
 * \return IVP parameters in CVode data
 */
extern const MPT_IVP_STRUCT(parameters) *_mpt_sundials_cvode_parameters(const MPT_SOLVER_STRUCT(cvode) *cv)
{
	return &cv->ivp;
}

/*!
 * \ingroup mptSundialsCVode
 * \brief CVode data destruction
 * 
 * Destroy allocated CVode solver data.
 * 
 * \param cv  CVode solver data pointer
 * 
 * \return current time value of CVode data
 */
extern double _mpt_sundials_cvode_time(const MPT_SOLVER_STRUCT(cvode) *cv)
{
	return cv->t;
}

/*!
 * \ingroup mptSundialsCVode
 * \brief CVode data destruction
 * 
 * Destroy allocated CVode solver data.
 * 
 * \param cv  CVode solver data pointer created by _mpt_sundials_cvode_create()
 */
extern void _mpt_sundials_cvode_destroy(MPT_SOLVER_STRUCT(cvode) *cv)
{
	if (cv) {
		mpt_sundials_cvode_fini(cv);
		free(cv);
	}
}
