/*!
 * get grid start and dimension.
 */

#include <errno.h>

#include "array.h"
#include "meta.h"

#include "values.h"

/*!
 * \ingroup mptValues
 * \brief create grid data
 * 
 * Create grid values from configuration.
 * 
 * \param grid  target array
 * \param conf  configuration node for grid
 * 
 * \return size of grid
 */
extern int mpt_conf_grid(MPT_STRUCT(array) *grid, const MPT_INTERFACE(metatype) *conf)
{
	MPT_INTERFACE(iterator) *it, *nit;
	MPT_STRUCT(buffer) *buf;
	double *dest;
	const char *desc;
	size_t old, len;
	int ret, pts;
	
	if ((buf = grid->_buf)) {
		old = buf->_used;
	} else {
		old = 0;
	}
	if (!conf) {
		nit = it = mpt_iterator_linear(10, 0, 1);
	}
	/* use existing iterator */
	else if ((ret = conf->_vptr->conv(conf, MPT_ENUM(TypeIterator), &it)) > 0 && it) {
		nit = 0;
	}
	/* use default grid settings */
	else if (!(desc = mpt_meta_data(conf, 0))) {
		nit = it = mpt_iterator_linear(10, 0, 1);
	}else {
		nit = it = mpt_iterator_create(desc);
	}
	if (!it) {
		return MPT_ERROR(BadValue);
	}
	pts = 0;
	len = 0;
	ret = -1;
	while (1) {
		/* fill buffer data */
		while (len) {
			/* get iterator data */
			if ((ret = it->_vptr->get(it, 'd', dest)) < 0) {
				if (nit) {
					nit->_vptr->ref.unref((void *) it);
				}
				buf->_used = old;
				return ret;
			}
			if (!ret) {
				break;
			}
			/* advance position and iterator */
			--len;
			++pts;
			++dest;
			if ((ret = it->_vptr->advance(it)) < 0) {
				if (nit) {
					nit->_vptr->ref.unref((void *) it);
				}
				buf->_used = old;
				return ret;
			}
			if (!ret) {
				break;
			}
		}
		/* increase buffer size */
		if (!(dest = mpt_values_prepare(grid, len = 16))) {
			if (buf) buf->_used = old;
			return MPT_ERROR(BadOperation);
		}
	}
	if (nit) {
		nit->_vptr->ref.unref((void *) it);
	}
	/* crop unused trailing buffer data */
	if (buf && len) {
		buf->_used -= len * sizeof(double);
	}
	return pts;
}

