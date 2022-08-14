/*!
 * MPT solver library
 *   set nonlinear solver parameter from configuration list
 */

#include <stdio.h>
#include <limits.h>
#include <errno.h>

#include "convert.h"
#include "node.h"
#include "output.h"

#include "values.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief configure NLS data
 * 
 * Get data for nonlinear solver form configuration.
 * 
 * \param md   client data descriptor
 * \param conf client configuration list
 * \param out  error log descriptor
 * 
 * \return parameter cont
 */
extern int mpt_conf_nls(MPT_STRUCT(solver_data) *md, const MPT_STRUCT(node) *conf, MPT_INTERFACE(logger) *out)
{
	MPT_STRUCT(node) *node;
	const char *data;
	int len, np = 0;
	
	/* read parameters and boundary conditions */
	if ((node = mpt_node_next(conf, "param"))) {
		if ((np = mpt_conf_param(&md->param, node, 3)) < 0) {
			if (out) mpt_log(out, __func__, MPT_LOG(Error), "%s: %i",
			                 MPT_tr("error in initial parameters"), np);
			return np;
		}
		md->npar = np;
	}
	/* read parameter count */
	else if ((node = mpt_node_next(conf, "npar"))) {
		if (!(data = mpt_node_data(conf, 0)) || (len = mpt_cint(&np, data, 0, 0)) <= 0 || np <= 0 || np > len) {
			if (out) mpt_log(out, __func__, MPT_LOG(Error), "%s: %i",
			                 MPT_tr("invalid explicit parameter count"), np);
			return MPT_ERROR(BadValue);
		}
		md->npar = np;
	}
	if (md->nval) {
		if (md->nval < md->npar) {
			if (out) mpt_log(out, __func__, MPT_LOG(Error), "%s: %i",
			                 MPT_tr("invalid existing parameter count"), np);
			
		}
		return 0;
	}
	/* read experimental data to user parameter matrix */
	if ((node = mpt_node_next(conf, "data"))) {
		FILE *fd;
		double *val;
		int nd;
		
		if (!(data = mpt_node_data(node, 0))) {
			if (out) mpt_log(out, __func__, MPT_LOG(Error), "%s: %s",
			                 MPT_tr("invalid exdata filename"), "<null>");
			return MPT_ERROR(BadValue);
		}
		if (!(fd = fopen(data, "r"))) {
			if (out) mpt_log(out, __func__, MPT_LOG(Error), "%s: %s",
			                 MPT_tr("unable to open data file"), data);
			return MPT_ERROR(BadValue);
		}
		if (fscanf(fd, "%d%d%*[^\n]", &len, &nd) != 2 || len < 1 || nd < 1 || INT_MAX/len < (nd+1)) {
			fclose(fd);
			if (out) mpt_log(out, __func__, MPT_LOG(Error), "%s",
			                 MPT_tr("unable to get user data dimensions"));
			return MPT_ERROR(BadType);
		}
		if (len < md->npar) {
			fclose(fd);
			if (out) mpt_log(out, __func__, MPT_LOG(Error), "%s: %i < %i",
			                 MPT_tr("initial value count too low"), len, md->npar);
			return MPT_ERROR(BadValue);
		}
		if (!(val = mpt_values_prepare(&md->val, len*nd))) {
			fclose(fd);
			if (out) mpt_log(out, __func__, MPT_LOG(Error), "%s",
			                 MPT_tr("unable to reserve residual data"));
			return MPT_ERROR(BadOperation);
		}
		if (mpt_values_file(fd, len, nd, val) < 0) {
			fclose(fd);
			if (out) mpt_log(out, __func__, MPT_LOG(Error), "%s",
			                 MPT_tr("error while reading user data"));
			return MPT_ERROR(MissingData);
		}
		fclose(fd);
		md->nval = len;
		return nd;
	}
	/* explicit residual settings */
	else if ((node = mpt_node_next(conf, "nres"))) {
		int nr, len;
		if (!(data = mpt_node_data(node, 0))) {
			if (out) mpt_log(out, __func__, MPT_LOG(Error), "%s",
			                 MPT_tr("invalid residual count format"));
			return MPT_ERROR(BadType);
		}
		if ((len = mpt_cint(&nr, data, 0, 0)) <= 0 || nr < md->npar) {
			if (out) mpt_log(out, __func__, MPT_LOG(Error), "%s: %i < %i",
			                 MPT_tr("invalid residual count"), nr, md->npar);
			return MPT_ERROR(BadValue);
		}
		md->nval = nr;
	}
	/* default to nonlinear equotation */
	else {
		md->nval = 0;
	}
	return 0;
}

