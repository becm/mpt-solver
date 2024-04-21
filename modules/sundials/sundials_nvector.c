/*!
 * Sundials N_Vector creation
 */

#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <nvector/nvector_serial.h>
#pragma GCC diagnostic pop

#include "sundials.h"

#if SUNDIALS_VERSION_MAJOR >= 6
extern N_Vector mpt_sundials_nvector(sunindextype len, SUNContext ctx)
{
	return N_VNew_Serial(len, ctx);
}
#else
extern N_Vector mpt_sundials_nvector(sunindextype len)
{
	return N_VNew_Serial(len);
}
#endif
