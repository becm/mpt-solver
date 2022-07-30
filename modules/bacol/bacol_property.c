/*!
 * set BACOL solver parameter from input source
 */

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#include "version.h"

#include "meta.h"

#include "bacol.h"

static int setBackend(MPT_SOLVER_STRUCT(bacol) *data, MPT_INTERFACE(convertable) *src)
{
	char *val = 0;
	int len = 0;
	
	if (src && (len = src->_vptr->convert(src, 'k', &val)) < 0) {
		return len;
	}
	if (mpt_bacol_backend(data, val) < 0) {
		return MPT_ERROR(BadValue);
	}
	return 0;
}

/*!
 * \ingroup mptBacol
 * \brief set BACOL property
 * 
 * Change property of BACOL solver
 * 
 * \param bac  BACOL data
 * \param name name of property to change
 * \param src  data source to change property
 * 
 * \retval <0  failure
 * \retval >=0 consumed value count
 */
extern int mpt_bacol_set(MPT_SOLVER_STRUCT(bacol) *bac, const char *name, MPT_INTERFACE(convertable) *src)
{
	int ret = 0;
	
	if (!name) {
		double t = 0;
		if (src && (ret = src->_vptr->convert(src, 'd', &t)) < 0) {
			return ret;
		}
		bac->t = t;
		bac->mflag.noinit = -1;
		return ret;
	}
	if (!*name) {
		MPT_IVP_STRUCT(parameters) ivp = MPT_IVPPAR_INIT;
		
		if (src && (ret =  mpt_solver_module_ivpset(&ivp, src)) < 0) {
			return ret;
		}
		mpt_bacol_fini(bac);
		mpt_bacol_init(bac);
		bac->ivp = ivp;
		
		return ret;
	}
	if (!strcasecmp(name, "atol")) {
		return mpt_solver_module_tol_set(&bac->atol, src, __MPT_IVP_ATOL);
	}
	if (!strcasecmp(name, "rtol")) {
		return mpt_solver_module_tol_set(&bac->rtol, src, __MPT_IVP_RTOL);
	}
	/* no interaction after prepare */
	if (bac->mflag.noinit >= 0) {
		return MPT_ERROR(BadOperation);
	}
	/* bacol parameters */
	if (!strcasecmp(name, "kcol")) {
		int32_t kcol = 2;
	
		if (src && (ret = src->_vptr->convert(src, 'i', &kcol)) < 0) {
			return ret;
		}
		else if (kcol < 2 || kcol > 10) {
			return MPT_ERROR(BadValue);
		}
		bac->kcol = kcol;
		return 0;
	}
	if (!strcasecmp(name, "nint")) {
		int32_t nint = 10;
		
		/* initial grid value size fixed after init */
		if (bac->xy) {
			return MPT_ERROR(BadOperation);
		}
		if (src && (ret = src->_vptr->convert(src, 'i', &nint)) < 0) {
			return ret;
		}
		if (nint < 2 || nint > bac->nintmx) {
			return MPT_ERROR(BadValue);
		}
		bac->nint = nint;
		return 0;
	}
	if (!strcasecmp(name, "nintmx")) {
		int32_t nintmx = 127;
		
		if (src && (ret = src->_vptr->convert(src, 'i', &nintmx)) < 0) {
			return ret;
		}
		if (nintmx < bac->nint) {
			return MPT_ERROR(BadValue);
		}
		bac->nintmx = nintmx;
		return 0;
	}
	if (!strncasecmp(name, "dirichlet", 3)) {
		int32_t bdir = 0;
		if (src && (ret = src->_vptr->convert(src, 'i', &bdir) < 0)) {
			return ret;
		}
		bac->mflag.bdir = bdir;
		return 0;
	}
	/* ode parameter */
	if (!strcasecmp(name, "stepinit") || !strcasecmp(name, "initstep")) {
		if (src && (ret = src->_vptr->convert(src, 'd', &bac->initstep)) < 0) {
			return ret;
		}
		bac->mflag.step = ret ? 1 : 0;
		return 0;
	}
	/* solving backend */
	if (!strcasecmp(name, "backend")) {
		return setBackend(bac, src);
	}
#ifdef MPT_BACOL_DASSL
	/* dassl parameter */
	if (bac->_backend == 'd' || bac->_backend == 'D') {
	if (!strcasecmp(name, "tstop")) {
		if (src && (ret = src->_vptr->convert(src, 'd', &bac->bd.tstop)) < 0) {
			return ret;
		}
		bac->mflag.tstop = ret ? 1 : 0;
		return 0;
	}
	/* dassl parameter */
	if (!strcasecmp(name, "maxstep")) {
		int32_t ms = 0;
		if (src && (ret = src->_vptr->convert(src, 'i', &ms)) < 0) {
			return ret;
		}
		bac->mflag.mstep = ms;
		return 0;
	}
	if (!strcasecmp(name, "dasslbdf") || !strcasecmp(name, "bdf")) {
		int32_t dbmax = 0;
		if (src && (ret = src->_vptr->convert(src, 'i', &dbmax)) < 0) {
			return ret;
		}
		bac->mflag.dbmax = dbmax;
		return 0;
	}
	}
#endif
	return MPT_ERROR(BadArgument);
}

/*!
 * \ingroup mptBacol
 * \brief get BACOL property
 * 
 * Get property of BACOL solver
 * 
 * \param bac  BACOL data
 * \param prop object property
 * 
 * \retval 0   default value
 * \retval <0  failure
 * \retval >0  changed property
 */
extern int mpt_bacol_get(const MPT_SOLVER_STRUCT(bacol) *bac, MPT_STRUCT(property) *prop)
{
	const char *name;
	intptr_t pos = -1, id;
	
	if (!prop) {
		return MPT_SOLVER_ENUM(PDE);
	}
	if (!(name = prop->name)) {
		if ((pos = (intptr_t) prop->desc) < 0) {
			return MPT_ERROR(BadArgument);
		}
	}
	else if (!*name) {
		prop->name = "bacol";
		prop->desc = "SPline PDE solver";
		return mpt_solver_module_value_ivp(prop, bac ? &bac->ivp : 0);
	}
	else if (!strcasecmp(name, "version")) {
		static const char version[] = BUILD_VERSION"\0";
		prop->name = "version";
		prop->desc = "solver release information";
		mpt_solver_module_value_string(prop, version);
		return 0;
	}
	
	id = -1;
	if (name ? !strcasecmp(name, "atol") : pos == ++id) {
		prop->name = "atol";
		prop->desc = "absolute tolerances";
		if (!bac) {
			MPT_value_set(&prop->val, 'd', 0);
			return id;
		}
		return mpt_solver_module_tol_get(prop, &bac->atol);
	}
	if (name ? !strcasecmp(name, "rtol") : pos == ++id) {
		prop->name = "rtol";
		prop->desc = "relative tolerances";
		if (!bac) {
			MPT_value_set(&prop->val, 'd', 0);
			return id;
		}
		return mpt_solver_module_tol_get(prop, &bac->rtol);
	}
	/* bacol parameters */
	if (name ? !strcasecmp(name, "kcol") : pos == ++id) {
		prop->name = "kcol";
		prop->desc = "collocations per interval";
		if (!bac) {
			MPT_value_set(&prop->val, 'n', 0);
			return id;
		}
		mpt_solver_module_value_set(prop, 'n', &bac->kcol, sizeof(bac->kcol));
		return bac->kcol == 2 ? 0 : 1;
	}
	if (name ? !strcasecmp(name, "nint") : pos == ++id) {
		prop->name = "nint";
		prop->desc = "number of internal intervals";
		if (!bac) {
			MPT_value_set(&prop->val, 'i', 0);
			return id;
		}
		mpt_solver_module_value_int(prop, &bac->nint);
		return bac->nint == 10 ? 0 : 1;
	}
	if (name ? (!strcasecmp(name, "nout") || !strcasecmp(name, "iout")) : pos == ++id) {
		prop->name = "iout";
		prop->desc = "number of output intervals";
		if (!bac) {
			MPT_value_set(&prop->val, 'u', 0);
			return id;
		}
		mpt_solver_module_value_set(prop, 'u', &bac->ivp.pint, sizeof(bac->ivp.pint));
		return bac->ivp.pint != 10 ? 1 : 0;
	}
	if (name ? !strcasecmp(name, "nintmx") : pos == ++id) {
		prop->name = "nintmx";
		prop->desc = "maximum internal intervals";
		if (!bac) {
			MPT_value_set(&prop->val, 'i', 0);
			return id;
		}
		mpt_solver_module_value_int(prop, &bac->nintmx);
		return bac->nintmx == MPT_BACOL_NIMAXDEF ? 0 : 1;
	}
	if (name ? !strcasecmp(name, "dirichlet") : pos == ++id) {
		prop->name = "dirichlet";
		prop->desc = "boundaries of dirichlet type";
		if (!bac) {
			MPT_value_set(&prop->val, 'i', 0);
			return id;
		}
		return mpt_solver_module_value_int(prop, &bac->mflag.bdir);
	}
	/* ode parameter */
	if (name ? !strcasecmp(name, "stepinit") : pos == ++id) {
		prop->name = "stepinit";
		prop->desc = "expl. initial stepsize";
		if (!bac) {
			MPT_value_set(&prop->val, 'i', 0);
			return id;
		}
		mpt_solver_module_value_double(prop, &bac->initstep);
		return bac->mflag.step ? 1 : 0;
	}
	/* solving backend */
	if (name ? !strcasecmp(name, "backend") : pos == ++id) {
		const char *val;
		prop->name = "backend";
		prop->desc = "solver step backend";
		if (!bac) {
			MPT_value_set(&prop->val, 's', 0);
			return id;
		}
		switch (bac->_backend) {
#ifdef MPT_BACOL_DASSL
		  case 'd': case 'D': val = "dassl"; break;
#endif
#ifdef MPT_BACOL_RADAU
		  case 'r': case 'R': val = "radau"; break;
#endif
		  default: val = "<unknown>";
		}
		mpt_solver_module_value_string(prop, val);
		return id;
	}
#ifdef MPT_BACOL_DASSL
	/* dassl parameter */
	if (!bac || bac->_backend == 'd' || bac->_backend == 'D') {
	if (name ? (!strcasecmp(name, "tstop") || !strcasecmp(name, "mflag(3)")) : pos == ++id) {
		prop->name = "tstop";
		prop->desc = "max. time allowed";
		if (!bac) {
			MPT_value_set(&prop->val, 'd', 0);
			return id;
		}
		mpt_solver_module_value_rvec(prop, 1, &bac->rpar);
		return bac->mflag.tstop ? 1 : 0;
	}
	/* dassl parameter */
	if (name ? (!strcasecmp(name, "maxstep") || !strcasecmp(name, "mflag(4)")) : pos == ++id) {
		prop->name = "maxstep";
		prop->desc = "max. steps per call";
		if (!bac) {
			MPT_value_set(&prop->val, 'i', 0);
			return id;
		}
		mpt_solver_module_value_ivec(prop, 8, &bac->ipar);
		return bac->mflag.mstep ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "dasslbdf") || !strcasecmp(name, "bdf") || !strcasecmp(name, "mflag(7)")) : pos == ++id) {
		prop->name = "dasslbdf";
		prop->desc = "max. number of BDF order";
		if (!bac) {
			MPT_value_set(&prop->val, 'i', 0);
			return id;
		}
		mpt_solver_module_value_ivec(prop, 15, &bac->ipar);
		return bac->mflag.dbmax ? 1 : 0;
	}
	}
#endif
	return MPT_ERROR(BadArgument);
}
