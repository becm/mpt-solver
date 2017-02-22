/*!
 * append profile data to buffer
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "node.h"

#include "values.h"

#include "solver.h"

/*!
 * \ingroup mptValues
 * \brief configure parameter data
 * 
 * Get profile data from configuration element.
 * 
 * \param arr  target array descriptor
 * \param len  length of profiles
 * \param neqs number of profiles
 * \param conf profile configuration node
 * \param out  error log descriptor
 * 
 * \return zero on success
 */
extern int mpt_conf_profiles(int len, double *val, int neqs, const MPT_STRUCT(node) *conf, const double *grid, MPT_INTERFACE(logger) *out)
{
	const MPT_STRUCT(node) *prof;
	const char *desc;
	int err, adv = 1, ld;

	if (len < 1 || !(ld = neqs)) {
		(void) mpt_log(out, __func__, MPT_LOG(Error), "%s: (len = %i) > (neqs = %i)",
		               MPT_tr("bad solver data size"), len, neqs);
		return MPT_ERROR(BadArgument);
	}
	if (neqs < 0) {
		ld  = 1;
		adv = len;
		neqs = -neqs;
	}
	if (!(prof = conf) || (prof = conf->children) || !(desc = mpt_node_data(conf, 0))) {
		int np = 0;
		
		while (prof) {
			int err;
			
			if ((err = mpt_conf_profile(len, val, ld, prof, grid)) < 0) {
				if (out) mpt_log(out, __func__, MPT_LOG(Error), "%s: %d",
				                 MPT_tr("bad profile"), np+1);
				return np;
			}
			val += adv;
			if (++np == neqs) {
				return np;
			}
			prof = prof->next;
		}
		if (np < neqs && out) {
			if (!np) {
				mpt_log(out, __func__, MPT_LOG(Warning), "%s",
			                MPT_tr("use zero initial values"));
			}
			else if ((neqs - np) < 2) {
				mpt_log(out, __func__, MPT_LOG(Warning), "%s (%d -> %d)",
				        MPT_tr("reuse initial values"), np, neqs);
			} else {
				mpt_log(out, __func__, MPT_LOG(Warning), "%s (%d -> %d..%d)",
				        MPT_tr("reuse initial values"), np, np+1, neqs);
			}
			(void) mpt_values_bound(len, val, ld, 0., 0., 0.);
			val += adv;
			
			err = np+1;
			while (err++ < neqs) {
				mpt_copy64(len, val-adv, ld, val, ld);
				val += adv;
			}
		}
		return np;
	}
	if (neqs > 1) {
		if (out) mpt_log(out, __func__, MPT_LOG(Error), "%s",
		                 MPT_tr("PDE needs verbose profile configuration"));
		return MPT_ERROR(BadType);
	}
	if ((err = mpt_valtype_select(&desc)) < 0) {
		if (out) mpt_log(out, __func__, MPT_LOG(Error), "%s: %s",
		                 MPT_tr("bad profile description"), desc);
		return MPT_ERROR(BadValue);
	}
	else if (mpt_valtype_init(err, desc, len, val, neqs, grid) < 0) {
		(void) mpt_log(out, __func__, MPT_LOG(Error), "%s: %s",
		               MPT_tr("bad profile parameter"), desc);
		return MPT_ERROR(BadOperation);
	}
	return 1;
}

