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

static int setBackend(MPT_SOLVER_STRUCT(bacol) *data, const MPT_INTERFACE(metatype) *src)
{
	char *val = 0;
	int len = 0;
	
	if (src && (len = src->_vptr->conv(src, 'k', &val)) < 0) {
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
extern int mpt_bacol_set(MPT_SOLVER_STRUCT(bacol) *bac, const char *name, const MPT_INTERFACE(metatype) *src)
{
	int ret = 0;
	
	if (!name) {
		double t = 0;
		if (src && (ret = src->_vptr->conv(src, 'd', &t)) < 0) {
			return ret;
		}
		bac->t = t;
		bac->mflag.noinit = -1;
		return ret;
	}
	if (!*name) {
		MPT_IVP_STRUCT(parameters) ivp = MPT_IVPPAR_INIT;
		
		if (src && (ret =  mpt_solver_ivpset(&ivp, src)) < 0) {
			return ret;
		}
		mpt_bacol_fini(bac);
		mpt_bacol_init(bac);
		bac->ivp = ivp;
		
		return ret;
	}
	if (!strcasecmp(name, "atol")) {
		return mpt_solver_tol_set(&bac->atol, src, __MPT_IVP_ATOL);
	}
	if (!strcasecmp(name, "rtol")) {
		return mpt_solver_tol_set(&bac->rtol, src, __MPT_IVP_RTOL);
	}
	/* no interaction after prepare */
	if (bac->mflag.noinit >= 0) {
		return MPT_ERROR(BadOperation);
	}
	/* bacol parameters */
	if (!strcasecmp(name, "kcol")) {
		int32_t kcol = 2;
	
		if (src && (ret = src->_vptr->conv(src, 'i', &kcol)) < 0) {
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
		if (src && (ret = src->_vptr->conv(src, 'i', &nint)) < 0) {
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
		
		if (src && (ret = src->_vptr->conv(src, 'i', &nintmx)) < 0) {
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
		if (src && (ret = src->_vptr->conv(src, 'i', &bdir) < 0)) {
			return ret;
		}
		bac->mflag.bdir = bdir;
		return 0;
	}
	/* ode parameter */
	if (!strcasecmp(name, "stepinit") || !strcasecmp(name, "initstep")) {
		if (src && (ret = src->_vptr->conv(src, 'd', &bac->initstep)) < 0) {
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
		if (src && (ret = src->_vptr->conv(src, 'd', &bac->bd.tstop)) < 0) {
			return ret;
		}
		bac->mflag.tstop = ret ? 1 : 0;
		return 0;
	}
	/* dassl parameter */
	if (!strcasecmp(name, "maxstep")) {
		int32_t ms = 0;
		if (src && (ret = src->_vptr->conv(src, 'i', &ms)) < 0) {
			return ret;
		}
		bac->mflag.mstep = ms;
		return 0;
	}
	if (!strcasecmp(name, "dasslbdf") || !strcasecmp(name, "bdf")) {
		int32_t dbmax = 0;
		if (src && (ret = src->_vptr->conv(src, 'i', &dbmax)) < 0) {
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
		prop->name = "bacol"; prop->desc = "SPline PDE solver";
		prop->val.fmt = "ii"; prop->val.ptr = &bac->ivp;
		return (bac->ivp.pint || bac->ivp.neqs != 1) ? 1 : 0;
	}
	else if (!strcasecmp(name, "version")) {
		static const char version[] = BUILD_VERSION"\0";
		prop->name = "version"; prop->desc = "solver release information";
		prop->val.fmt = 0; prop->val.ptr = version;
		return 0;
	}
	
	id = -1;
	if (name ? !strcasecmp(name, "atol") : pos == ++id) {
		prop->name = "atol"; prop->desc = "absolute tolerances";
		if (!bac) {
			prop->val.fmt = 0; prop->val.ptr = &bac->atol;
			return id;
		}
		return mpt_solver_tol_get(&bac->atol, &prop->val);
	}
	if (name ? !strcasecmp(name, "rtol") : pos == ++id) {
		prop->name = "rtol"; prop->desc = "relative tolerances";
		if (!bac) {
			prop->val.fmt = 0; prop->val.ptr = &bac->rtol;
			return id;
		}
		return mpt_solver_tol_get(&bac->rtol, &prop->val);
	}
	/* bacol parameters */
	if (name ? !strcasecmp(name, "kcol") : pos == ++id) {
		prop->name = "kcol"; prop->desc = "collocations per interval";
		prop->val.fmt = "n"; prop->val.ptr = &bac->kcol;
		if (!bac) return id;
		return bac->kcol == 2 ? 0 : 1;
	}
	if (name ? !strcasecmp(name, "nint") : pos == ++id) {
		prop->name = "nint"; prop->desc = "number of internal intervals";
		prop->val.fmt = "i"; prop->val.ptr = &bac->nint;
		if (!bac) return id;
		return bac->nint == 10 ? 0 : 1;
	}
	if (name ? (!strcasecmp(name, "nout") || !strcasecmp(name, "iout")) : pos == ++id) {
		prop->name = "iout"; prop->desc = "number of internal intervals";
		prop->val.fmt = "i"; prop->val.ptr = &bac->ivp.pint;
		if (!bac) return id;
		return bac->ivp.pint ? 1 : 0;
	}
	if (name ? !strcasecmp(name, "nintmx") : pos == ++id) {
		prop->name = "nintmx"; prop->desc = "maximum internal intervals";
		prop->val.fmt = "i"; prop->val.ptr = &bac->nintmx;
		if (!bac) return id;
		return bac->nintmx == MPT_BACOL_NIMAXDEF ? 0 : 1;
	}
	if (name ? !strcasecmp(name, "dirichlet") : pos == ++id) {
		prop->name = "dirichlet"; prop->desc = "boundaries of dirichlet type";
		prop->val.fmt = "i"; prop->val.ptr = &bac->mflag.bdir;
		if (!bac) return id;
		return bac->mflag.bdir;
	}
	/* ode parameter */
	if (name ? !strcasecmp(name, "stepinit") : pos == ++id) {
		prop->name = "stepinit"; prop->desc = "expl. initial stepsize";
		prop->val.fmt = "d"; prop->val.ptr = &bac->initstep;
		if (!bac) return id;
		return bac->mflag.step;
	}
	/* solving backend */
	if (name ? !strcasecmp(name, "backend") : pos == ++id) {
		prop->name = "backend"; prop->desc = "solver step backend";
		prop->val.fmt = 0; prop->val.ptr = 0;
		if (!bac) return id;
		switch (bac->_backend) {
#ifdef MPT_BACOL_DASSL
		  case 'd': case 'D': prop->val.ptr = "dassl"; break;
#endif
#ifdef MPT_BACOL_RADAU
		  case 'r': case 'R': prop->val.ptr = "radau"; break;
#endif
		  default: prop->val.ptr = "<unknown>";
		}
		return id;
	}
#ifdef MPT_BACOL_DASSL
	/* dassl parameter */
	if (!bac || bac->_backend == 'd' || bac->_backend == 'D') {
	if (name ? !strcasecmp(name, "tstop") : pos == ++id) {
		prop->name = "tstop"; prop->desc = "max. time allowed";
		prop->val.fmt = "d"; prop->val.ptr = &bac->mflag.bdir;
		if (!bac) return id;
		return id;
	}
	/* dassl parameter */
	if (name ? !strcasecmp(name, "maxstep") : pos == ++id) {
		prop->name = "maxstep"; prop->desc = "max. steps per call";
		prop->val.fmt = "d"; prop->val.ptr = &bac->bd.tstop;
		if (!bac) return id;
		return bac->mflag.tstop;
	}
	if (name ? (!strcasecmp(name, "dasslbdf") || !strcasecmp(name, "bdf")) : pos == ++id) {
		prop->name = "dasslbdf"; prop->desc = "max. number of BDF order";
		prop->val.fmt = "i"; prop->val.ptr = &bac->mflag.bdir;
		if (!bac) return id;
		return bac->mflag.bdir ? 1 : 0;
	}
	}
#endif
	return MPT_ERROR(BadArgument);
}
