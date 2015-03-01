/*!
 * Sundials N_Vector creation
 */

#include <errno.h>

#include <nvector/nvector_serial.h>

#include "sundials.h"

extern N_Vector sundials_nvector_empty(long len)
{
	return N_VNewEmpty_Serial(len);
}
