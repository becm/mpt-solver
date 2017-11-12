/*!
 * Sundials N_Vector creation
 */

#include <nvector/nvector_serial.h>

#include "sundials.h"

extern N_Vector sundials_nvector_new(long len)
{
	return N_VNew_Serial(len);
}
