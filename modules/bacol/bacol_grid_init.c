/*!
 * initialize BACOL solver grid
 */

#include <string.h>

#include "bacol.h"

extern int mpt_bacol_grid_init(int vint, const double *val, int xint, double *x)
{
	if (vint < 0 || xint < 1) {
		return MPT_ERROR(BadArgument);
	}
	if (vint < 3) {
		double min, dx;
		int i;
		
		min = (vint > 0) ? val[0] : 0;
		dx  = (vint > 1) ? val[1] : min + 1.;
		
		if (!dx || !(dx /= xint)) {
			return MPT_ERROR(BadValue);
		}
		for (i = 0; i <= xint; ++i) {
			x[i] = min + i * dx;
		}
		return xint;
	}
	/* dimension less or equal to initial intervals */
	if (vint < xint) {
		double dv, dx, base;
		int i, j;
		
		base = val[0];
		dv = (val[vint] - base);
		dx = dv / xint;
		dv /= vint;
		x[0] = base;
		i = j = 1;
		while (i <= vint) {
			double next = base + j * dx;
			
			/* accept current value */
			if (val[i] <= next || xint <= vint) {
				x[j++] = val[i++];
				continue;
			}
			/* insert intermediate */
			xint--;
			x[j++] = next;
		}
		return j-1;
	}
	/* more data than initial intervals */
	else {
		int pos = 1;
		x[0] = val[0];
		x[xint] = val[vint--];
		
		while (pos < xint && vint) {
			double sum = 0.0;
			int i, count = vint / (xint-pos);
			
			for (i = 0; i < count; ++i) sum += val[i];
			val += count;
			vint -= count;
			x[pos++] = sum / count;
		}
		return xint;
	}
}

