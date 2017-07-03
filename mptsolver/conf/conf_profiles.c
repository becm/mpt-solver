/*!
 * append profile data to buffer
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "node.h"
#include "meta.h"
#include "convert.h"

#include "values.h"

#include "solver.h"

struct iterProfile
{
	MPT_INTERFACE(iterator) _it;
	double t;
	long pos;
	long len;
	int neqs;
};

static void iterProfileUnref(MPT_INTERFACE(unrefable) *ref)
{
	const struct iterProfile *p = (void *) (ref + 1);
	MPT_INTERFACE(iterator) **iptr = (void *) (p + 1);
	int i;
	
	for (i = 0; i < p->neqs; ++i) {
		MPT_INTERFACE(iterator) *it;
		if ((it = iptr[i])) {
			it->_vptr->ref.unref((void *) it);
		}
	}
	free(ref);
}
static int iterProfileGet(MPT_INTERFACE(iterator) *it, int type, void *dest)
{
	const struct iterProfile *p = (void *) (it + 1);
	MPT_INTERFACE(iterator) **iptr = (void *) (p + 1);
	double *tmp = (void *) (iptr + p->neqs);
	struct iovec *vec;
	int i;
	
	if (p->pos < 0) {
		if (type != 'd') {
			return MPT_ERROR(BadType);
		}
		if (dest) {
			*((double *) dest) = p->t;
		}
	}
	if (type != MPT_value_toVector('d')) {
		return MPT_ERROR(BadType);
	}
	if (p->pos >= p->len) {
		return 0;
	}
	if (!(vec = dest)) {
		return MPT_value_toVector('d');
	}
	for (i = 0; i < p->neqs; ++i) {
		int ret;
		
		if (!(it = iptr[i])) {
			continue;
		}
		if ((ret = it->_vptr->get(it, 'd', tmp + i)) < 0) {
			return ret;
		}
		if (!ret) {
			tmp[i] = 0;
		}
	}
	vec->iov_base = tmp;
	vec->iov_len = i * sizeof(*tmp);
	
	return MPT_value_toVector('d');
}
static int iterProfileAdvance(MPT_INTERFACE(iterator) *it)
{
	struct iterProfile *p = (void *) (it + 1);
	MPT_INTERFACE(iterator) **iptr = (void *) (p + 1);
	long i;
	
	if (p->pos < 0) {
		p->pos = 0;
		return MPT_value_toVector('d');
	}
	if (p->pos >= p->len) {
		return MPT_ERROR(BadOperation);
	}
	if (++p->pos == p->len) {
		return 0;
	}
	for (i = 0; i < p->neqs; ++i) {
		int ret;
		if ((it = iptr[i])
		 && (ret = it->_vptr->advance(it)) < 0) {
			return ret;
		}
	}
	return MPT_value_toVector('d');
}
static int iterProfileReset(MPT_INTERFACE(iterator) *it)
{
	struct iterProfile *p = (void *) (it + 1);
	MPT_INTERFACE(iterator) **iptr = (void *) (p + 1);
	long i;
	
	for (i = 0; i < p->neqs; ++i) {
		int ret;
		if ((it = iptr[i])
		 && (ret = it->_vptr->reset(it)) < 0) {
			return ret;
		}
	}
	p->pos = -1;
	return p->len + 1;
}
static const MPT_INTERFACE_VPTR(iterator) iterProfileCtl = {
	{ iterProfileUnref },
	iterProfileGet,
	iterProfileAdvance,
	iterProfileReset
};

/*!
 * \ingroup mptValues
 * \brief configure parameter data
 * 
 * Get profile data from configuration element.
 * 
 * \param dat  solver data descriptor
 * \param neqs number of profiles
 * \param t    initial time
 * \param conf profile configuration node
 * \param out  error log descriptor
 * 
 * \return iterator containing profile segments
 */
extern MPT_INTERFACE(iterator) *mpt_conf_profiles(const MPT_STRUCT(solver_data) *dat, int neqs, double t, const MPT_STRUCT(node) *conf, MPT_INTERFACE(logger) *out)
{
	struct iterProfile *ip;
	MPT_INTERFACE(iterator) **iptr;
	const MPT_STRUCT(node) *prof;
	const char *desc;
	int i;

	if ((i = dat->nval) < 1) {
		mpt_log(out, __func__, MPT_LOG(Error), "%s: (len = %i)",
		        MPT_tr("bad solver data size"), i);
		return 0;
	}
	if (neqs < 0) {
		neqs = 0;
		if (conf && (prof = conf->children)) {
			while (prof) {
				++neqs;
				prof = prof->next;
			}
		}
	}
	if (!(ip = malloc(sizeof(*ip) + neqs * (sizeof(*iptr) + sizeof(double))))) {
		mpt_log(out, __func__, MPT_LOG(Error), "%s: (len = %i)",
		        MPT_tr("failed to create iterator"), neqs);
		return 0;
	}
	iptr = (void *) (ip + 1);
	
	ip->_it._vptr = &iterProfileCtl;
	ip->t = t;
	ip->pos = -1;
	ip->len = dat->nval;
	ip->neqs = neqs;
	
	for (i = 0; i < neqs; ++i) {
		iptr[i] = 0;
	}
	/* no source iterators */
	if (!conf || !(prof = conf->children)) {
		double *val  = (void *) (iptr + neqs);
		i = 0;
		/* set static profile data */
		if (conf && (desc = mpt_node_data(conf, 0))) {
			while (i < neqs) {
				ssize_t len;
				if ((len = mpt_cdouble(val + i, desc, 0)) <= 0) {
					break;
				}
				desc += len;
				++i;
			}
		}
		while (i < neqs) {
			val[i] = 0;
		}
		return &ip->_it;
	}
	i = 0;
	while (prof && i < neqs) {
		desc = mpt_node_data(prof, 0);
		if (!(iptr[i++] = mpt_iterator_profile(&dat->val, desc))) {
			if (out) mpt_log(out, __func__, MPT_LOG(Error), "%s: %d",
			                 MPT_tr("bad profile"), i);
			iterProfileUnref((void *) &ip->_it);
			return 0;
		}
		prof = prof->next;
	}
	return &ip->_it;
}

