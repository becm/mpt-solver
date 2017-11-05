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

MPT_STRUCT(iterProfile) {
	MPT_INTERFACE(metatype) _mt;
	MPT_INTERFACE(iterator) _it;
	double t;
	long pos;
	long len;
	int neqs;
};
/* iterator interface */
static int iterProfileGet(MPT_INTERFACE(iterator) *it, int type, void *dest)
{
	MPT_STRUCT(iterProfile) *p = MPT_baseaddr(iterProfile, it, _it);
	MPT_INTERFACE(metatype) **mptr = (void *) (p + 1);
	MPT_INTERFACE(iterator) **iptr = (void *) (mptr + p->neqs);
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
		return 'd';
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
		MPT_INTERFACE(iterator) *curr;
		int ret;
		
		if (!(curr = iptr[i])) {
			continue;
		}
		if ((ret = curr->_vptr->get(curr, 'd', tmp + i)) < 0) {
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
	MPT_STRUCT(iterProfile) *p = MPT_baseaddr(iterProfile, it, _it);
	MPT_INTERFACE(metatype) **mptr = (void *) (p + 1);
	MPT_INTERFACE(iterator) **iptr = (void *) (mptr + p->neqs);
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
	MPT_STRUCT(iterProfile) *p = MPT_baseaddr(iterProfile, it, _it);
	MPT_INTERFACE(iterator) **mptr = (void *) (p + 1);
	MPT_INTERFACE(iterator) **iptr = (void *) (mptr + p->neqs);
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
/* reference interface */
static void iterProfileUnref(MPT_INTERFACE(reference) *ref)
{
	const MPT_STRUCT(iterProfile) *p = (void *) ref;
	MPT_INTERFACE(metatype) **mptr = (void *) (p + 1);
	int i;
	
	for (i = 0; i < p->neqs; ++i) {
		MPT_INTERFACE(metatype) *mt;
		if ((mt = mptr[i])) {
			mt->_vptr->ref.unref((void *) mt);
		}
	}
	free(ref);
}
static uintptr_t iterProfileRef(MPT_INTERFACE(reference) *ref)
{
	(void) ref;
	return 0;
}
/* metatype interface */
static int iterProfileConv(const MPT_INTERFACE(metatype) *mt, int type, void *ptr)
{
	const MPT_STRUCT(iterProfile) *p = (void *) mt;
	
	if (!type) {
		static const char fmt[] = { MPT_ENUM(TypeMeta), MPT_ENUM(TypeIterator), 0 };
		if (ptr) *((const char **) ptr) = fmt;
	}
	if (type == 'd') {
		if (ptr) *((double *) ptr) = p->t;
		return MPT_ENUM(TypeIterator);
	}
	if (type == MPT_ENUM(TypeMeta)) {
		if (ptr) *((const void **) ptr) = &p->_mt;
		return MPT_ENUM(TypeIterator);
	}
	if (type == MPT_ENUM(TypeIterator)) {
		if (ptr) *((const void **) ptr) = &p->_it;
		return MPT_ENUM(TypeMeta);
	}
	return MPT_ERROR(BadType);
}
static MPT_INTERFACE(metatype) *iterProfileClone(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}

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
extern MPT_INTERFACE(metatype) *mpt_conf_profiles(const MPT_STRUCT(solver_data) *dat, double t, const MPT_STRUCT(node) *conf, int neqs, MPT_INTERFACE(logger) *out)
{
	static const MPT_INTERFACE_VPTR(iterator) iterProfileIter = {
		iterProfileGet, iterProfileAdvance, iterProfileReset
	};
	static const MPT_INTERFACE_VPTR(metatype) iterProfileMeta = {
		{ iterProfileUnref, iterProfileRef }, iterProfileConv, iterProfileClone
	};
	MPT_STRUCT(iterProfile) *ip;
	MPT_INTERFACE(metatype) **mptr;
	MPT_INTERFACE(iterator) **iptr;
	const MPT_STRUCT(node) *prof;
	const char *desc;
	double *val;
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
	i = sizeof(*mptr) + sizeof(*iptr) + sizeof(*val);
	if (!(ip = malloc(sizeof(*ip) + neqs * i))) {
		mpt_log(out, __func__, MPT_LOG(Error), "%s: (len = %i)",
		        MPT_tr("failed to create iterator"), neqs);
		return 0;
	}
	mptr = memset(ip + 1,      0, neqs * sizeof(*mptr));
	iptr = memset(mptr + neqs, 0, neqs * sizeof(*iptr));
	val  = memset(iptr + neqs, 0, neqs * sizeof(*val));
	
	ip->_mt._vptr = &iterProfileMeta;
	ip->_it._vptr = &iterProfileIter;
	ip->t = t;
	ip->pos = -1;
	ip->len = dat->nval;
	ip->neqs = neqs;
	
	for (i = 0; i < neqs; ++i) {
		iptr[i] = 0;
	}
	for (i = 0; i < neqs; ++i) {
		val[i] = 0;
	}
	/* no source iterators */
	if (!conf || !(prof = conf->children)) {
		i = 0;
		/* set static profile data */
		if (conf && (desc = mpt_node_data(conf, 0))) {
			while (i < neqs) {
				ssize_t len;
				if ((len = mpt_cdouble(val + i++, desc, 0)) < 0) {
					if (out) mpt_log(out, __func__, MPT_LOG(Info), "%s: %d",
					                 MPT_tr("bad profile constant"), i);
					break;
				}
				if (!len) {
					break;
				}
				desc += len;
			}
		}
		return &ip->_mt;
	}
	while (prof && neqs--) {
		MPT_INTERFACE(metatype) *mt;
		if (!(desc = mpt_node_data(prof, 0))) {
			if (out) mpt_log(out, __func__, MPT_LOG(Info), "%s: %d",
			                 MPT_tr("no profile description"), i);
			continue;
		}
		if (!(mt = mpt_iterator_profile(&dat->val, desc))
		    && out) {
			mpt_log(out, __func__, MPT_LOG(Warning), "%s (%d): %s",
			        MPT_tr("bad profile"), i, desc);
		}
		*mptr++ = mt;
		*iptr = 0;
		if (mt->_vptr->conv(mt, MPT_ENUM(TypeIterator), iptr++) < 0) {
			if (out) {
				mpt_log(out, __func__, MPT_LOG(Warning), "%s (%d): %s",
				        MPT_tr("bad profile"), i, desc);
			}
		}
		prof = prof->next;
	}
	return &ip->_mt;
}

