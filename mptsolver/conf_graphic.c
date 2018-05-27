/*!
 * MPT solver library
 *   use node list data to setup layouts and mapping
 */

#include <string.h>

#include "node.h"
#include "output.h"

#include "values.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \~english
 * \brief set graphic parameters
 * 
 * Use graphic parameters from node configuration list.
 * 
 * \param out   output descriptor
 * \param conf  configuration list
 * \param log   error log target
 * 
 * \retval 0  success
 * \retval -2 error in layout open operation
 * \retval -3 error in bind operation
 */
extern int mpt_conf_graphic(MPT_INTERFACE(output) *out, const MPT_STRUCT(node) *conf, MPT_INTERFACE(logger) *log)
{
	MPT_STRUCT(node) *tmp;
	const char *data;
	int err;
	
	if (!conf || !out) {
		return 0;
	}
	if (!(tmp = mpt_node_next(conf, "layout")));
	/* assign layouts */
	else if (tmp->children) {
		const char *gen;
		tmp = tmp->children;
		do {
			if (!(gen = mpt_node_ident(tmp))) {
				continue;
			}
			if (!(data = mpt_node_data(tmp, 0))) {
				continue;
			}
			if (!strcmp(gen, "file")) {
				gen = 0;
			}
			if ((err = mpt_layout_open(out, data, gen)) < 0) {
				return -2;
			}
		} while ((tmp = tmp->next));
	}
	else if ((data = mpt_node_data(tmp, 0))) {
		if ((err = mpt_layout_open(out, data, 0)) < 0) {
			return -2;
		}
	}
	/* parse output mappings */
	if (!(tmp = mpt_node_next(conf, "graphic"))) {
		return 0;
	}
	if (tmp->children) {
		err = mpt_output_bind_list(out, tmp->children);
	}
	else if (!(data = mpt_node_data(tmp, 0))) {
		return 0;
	}
	else {
		err = mpt_output_bind_string(out, data);
	}
	if (!err) {
		return 0;
	}
	if (err < 0) {
		mpt_log(log, __func__, MPT_LOG(Warning), "%s",
		        MPT_tr("unable to apply graphic binding"));
		return err;
	}
	mpt_log(log, __func__, MPT_LOG(Error), "%s %i",
	        MPT_tr("error processing graphic binding"), err);
	return MPT_ERROR(BadValue);
}

