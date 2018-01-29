/*!
 * Sundials N_Vector creation
 */

#include <stdio.h>
#include <nvector/nvector_serial.h>

#include "sundials.h"

extern N_Vector mpt_sundials_nvector(sunindextype len)
{
	return N_VNew_Serial(len);
}
