/*!
 * MPT solver library
 *   push IVP history header
 */

#include "config.h"
#include "output.h"

#include "solver.h"

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
	static const char path[] = "mpt.graphic";
	MPT_INTERFACE(convertable) *val;
	int sep, off, ret;
	
	if (cfg) {
		off = 4;
		sep = 0;
	} else {
		off = 0;
		sep = '.';
	}
	ret = 0;
	if (!so->_data) {
		static const char path[] = "mpt.output";
		
		if ((val = mpt_config_get(cfg, path + off, sep, 0))) {
			val->_vptr->convert(val, MPT_ENUM(TypeOutputPtr), &so->_data);
			if (so->_data) {
				ret |= 0x1;
			}
		}
	}
	if (so->_graphic && so->_pass._buf) {
		return ret;
	}
	if ((val = mpt_config_get(cfg, path + off, sep, 0))) {
		const MPT_STRUCT(type_traits) *info;
		MPT_STRUCT(array) a = MPT_ARRAY_INIT;
		MPT_STRUCT(buffer) *buf;
		int type = 0;
		
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
	}
	return ret;
}
