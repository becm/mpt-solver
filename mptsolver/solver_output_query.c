/*!
 * MPT solver library
 *   push IVP history header
 */

#include "config.h"
#include "output.h"

#include "solver.h"

static int _apply_graphic(void *ctx, MPT_INTERFACE(convertable) *val, const MPT_INTERFACE(collection) *sub)
{
	MPT_STRUCT(solver_output) *so = ctx;
	const MPT_STRUCT(type_traits) *info;
	MPT_STRUCT(array) a = MPT_ARRAY_INIT;
	MPT_STRUCT(buffer) *buf;
	int type = 0;
	int ret = 0;
	
	(void) sub;
	
	if (!val) {
		return 0;
	}
	
	if (!so->_graphic) {
		val->_vptr->convert(val, MPT_ENUM(TypeOutputPtr), &so->_graphic);
		if (so->_graphic) {
			ret |= 0x2;
		}
	}
	if (so->_pass._buf) {
		return ret;
	}
	if (val->_vptr->convert(val, MPT_ENUM(TypeArray), &a) < 0
	 || !(buf = a._buf)) {
		return ret;
	}
	/* require raw or '(unsigned) byte' */
	if ((info = buf->_content_traits)
	 && ((info == mpt_type_traits('b'))
	  || (info == mpt_type_traits('y')))) {
		so->_pass._buf = buf;
		ret |= 0x8;
	} else {
		mpt_log(0, __func__, MPT_LOG(Warning), "%s: %d != %d",
		        MPT_tr("bad pass flags content type"), type, 'b');
		mpt_array_clone(&a, 0);
	}
	return ret;
}
/*!
 * \ingroup mptSolver
 * \brief get log target
 * 
 * Get logging descriptor from solver output components.
 * 
 * \param out  solver output descriptor
 * 
 * \return log descriptor
 */
extern int mpt_solver_output_query(MPT_STRUCT(solver_output) *so, const MPT_INTERFACE(config) *cfg)
{
	static const char path_graphic[] = "mpt.graphic";
	MPT_STRUCT(path) p = MPT_PATH_INIT;
	int off, ret, gr;
	
	if (cfg) {
		off = 4;
		p.sep = 0;
	} else {
		off = 0;
		p.sep = '.';
	}
	ret = 0;
	if (!so->_data) {
		static const char path_output[] = "mpt.output";
		mpt_path_set(&p, path_output + off, -1);
		
		if (mpt_config_getp(cfg, &p, MPT_ENUM(TypeOutputPtr), &so->_data) >= 0
		 && so->_data) {
			ret |= 0x1;
		}
	}
	if (so->_graphic && so->_pass._buf) {
		return ret;
	}
	
	mpt_path_set(&p, path_graphic + off, -1);
	if ((gr = mpt_config_query(cfg, &p, _apply_graphic, so)) >= 0) {
		ret |= gr;
	}
	mpt_path_set(&p, 0, 0);
	return ret;
}
