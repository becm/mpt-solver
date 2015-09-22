/*!
 * set nonlinear solver parameter from configuration list.
 */

#include <stdio.h>
#include <limits.h>
#include <errno.h>

#include "array.h"
#include "convert.h"
#include "node.h"

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
extern int mpt_conf_nls(MPT_SOLVER_STRUCT(data) *md, const MPT_STRUCT(node) *conf, MPT_INTERFACE(logger) *out)
{
	MPT_STRUCT(buffer) *buf;
	MPT_STRUCT(node) *node;
	const char *data;
	int	np, len;
	
	len = (buf = md->param._buf) ? buf->used / sizeof(double) : 0;
	np  = 0;
	
	/* read parameters and boundary conditions */
	if (!len && (node = mpt_node_next(conf, "param"))) {
		if ((len = mpt_conf_param(&md->param, node, 3)) < 0) {
			(void) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s: %i",
			               MPT_tr("error in initial parameters"), len);
			return -2;
		}
		md->npar = len;
	}
	/* read parameter count */
	if (!np) {
		if ((node = mpt_node_next(conf, "npar"))) {
			if (!(data = mpt_node_data(conf, 0)) || (len = mpt_cint(&np, data, 0, 0)) <= 0 || np <= 0 || np > len) {
				(void) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s: %i",
				               MPT_tr("invalid explicit parameter count"), np);
				return -3;
			}
		}
		np = md->npar;
	}
	if (md->nval) {
		/* check minimal parameter number */
		if (np > md->nval) {
			(void) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s: %i < %i",
			               MPT_tr("residual count too low"), len, np);
			return -2;
		}
	}
	/* read experimental data to user parameter matrix */
	else if ((node = mpt_node_next(conf, "exdata"))) {
		FILE	*fd;
		double	*val;
		int	nd;
		
		if (!(data = mpt_node_data(node, 0))) {
			(void) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s: %s",
			               MPT_tr("invalid exdata filename"), "<null>");
			return -3;
		}
		if (!(fd = fopen(data, "r"))) {
			(void) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s: %s",
			               MPT_tr("unable to open data file"), data);
			return -3;
		}
		if (fscanf(fd, "%d%d%*[^\n]", &len, &nd) != 2 || len < 1 || nd < 1 || INT_MAX/len < (nd+1)) {
			fclose(fd);
			(void) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s",
			               MPT_tr("unable to get user data dimensions"));
			return -4;
		}
		if (len < md->npar) {
			fclose(fd);
			(void) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s: %i < %i",
			               MPT_tr("initial value count too low"), len, md->npar);
			return -4;
		}
		if (!(val = mpt_values_prepare(&md->val, len*(nd+1)))) {
			fclose(fd);
			(void) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s",
			               MPT_tr("unable to reserve residual data"));
			return -4;
		}
		if (mpt_conf_file(fd, len, nd, val+len) < 0) {
			(void) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s",
			               MPT_tr("error while reading user data"));
			return -4;
		}
		md->nval = len;
	}
	/* explicit residual settings */
	else if ((node = mpt_node_next(conf, "nres"))) {
		int nr, len;
		if (!(data = mpt_node_data(node, 0))) {
			(void) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s",
			               MPT_tr("invalid residual count format"));
			return -2;
		}
		if ((len = mpt_cint(&nr, data, 0, 0)) <= 0 || nr < np) {
			(void) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s: %i < %i",
			               MPT_tr("invalid residual count"), nr, np);
			return -2;
		}
		if (!(data = (char*) mpt_values_prepare(&md->val, nr))) {
			(void) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s",
			               MPT_tr("unable to reserve residual data"));
			return -4;
		}
		md->nval = nr;
	}
	/* default to nonlinear equotation */
	else {
		if (!(data = (char*) mpt_values_prepare(&md->val, np))) {
			(void) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s",
			               MPT_tr("unable to reserve residual data"));
			return -4;
		}
		md->nval = np;
	}
	return np;
}

