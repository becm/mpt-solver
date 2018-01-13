/*!
 * initialize BACOL solver grid
 */

#include <string.h>

#include "bacol.h"

/*!
 * \ingroup mptBacol
 * \brief adapt grid
 * 
 * Linear interpolation for shrinking or growing reference grid.
 * 
 * \param      vint  input intervals
 * \param[in]  val   input grid data
 * \param      xint  desired output intervals
 * \param[out] x     output grid data
 * 
 * \retval <0  bad argument(s)
 * \retval >0  generated interval count
 */
extern int mpt_bacol_grid_init(int vint, const double *val, int xint, double *x)
{
	int left;
	if (!x || vint < 0 || xint < 1) {
		return MPT_ERROR(BadArgument);
	}
	if (!val) {
		double dx = 1.0 / xint;
		int i;
		for (i = 0; i <= xint; ++i) {
			x[i] = i * dx;
		}
		return xint;
	}
	if (vint < 2) {
		double min, dx;
		int i;
		
		min = val[0];
		dx  = vint ? val[1] : min + 1.;
		
		if (!dx || !(dx /= xint)) {
			return MPT_ERROR(BadValue);
		}
		for (i = 0; i <= xint; ++i) {
			x[i] = min + i * dx;
		}
		return xint;
	}
	x[0] = val[0];
	left = xint;
	
	/* destination larger */
	if (vint < xint) {
		double base = *val++;
		
		while (vint) {
			double dx, next = *val++;
			int i, curr;
			
			/* decreasing size towards end (vint -> left -> 0) */
			curr = 1;
			while (curr * vint < left) {
				++curr;
			}
			/* linear interpolation */
			dx = (next - base) / curr;
			for (i = 1; i < curr; ++i) {
				x[i] = base + i * dx;
			}
			/* final point / start of next segment */
			base = next;
			x[curr] = next;
			
			/* advance element */
			x += curr;
			left -= curr;
			--vint;
		}
		return xint;
	}
	/* operate on intermediate values */
	++val;
	--vint;
	++x;
	while (--left) {
		double sum = val[0];
		int i, count;
		
		/* increasing size towards end */
		count = vint / left;
		
		/* merge segment data */
		for (i = 1; i < count; ++i) {
			sum += val[i];
		}
		*(x++) = sum / count;
		
		/* advance source segment */
		val += count;
		vint -= count;
	}
	*x = *val;
	return xint;
}

