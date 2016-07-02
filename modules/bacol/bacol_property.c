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

static int setBackend(MPT_SOLVER_STRUCT(bacol) *data, MPT_INTERFACE(metatype) *src)
{
	char *val = 0;
	int len = 0;
	
	if (src && (len = src->_vptr->conv(src, 'k', &val)) < 0) {
		return len;
	}
	if (mpt_bacol_backend(data, val) < 0) {
		return MPT_ERROR(BadValue);
	}
	return len;
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
extern int mpt_bacol_set(MPT_SOLVER_STRUCT(bacol) *bac, const char *name, MPT_INTERFACE(metatype) *src)
{
	int len = 0;
	
	if (!name) {
		double t = bac->t;
		
		if (src && (len = src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), &t) <= 0)) {
			if (len < 0) {
				return len;
			}
			src = 0;
		}
		if (src && (len = mpt_vecpar_set(&bac->xy, bac->nint + 1, src)) <= 0) {
			if (len < 0) {
				return len;
			}
			mpt_bacol_grid_init(0, 0, bac->nint, bac->xy);
		}
		bac->mflag.noinit = -2;
		bac->t = t;
		return src ? len + 1 : 0;
	}
	if (!*name) {
		MPT_SOLVER_IVP_STRUCT(parameters) ivp = bac->ivp;
		
		if (src) {
			if ((len =  mpt_ivppar_set(&ivp, src)) < 0) {
				return len;
			}
			if (len < 2) {
				ivp.pint = -1;
			}
			else if (ivp.pint < 0) {
				return MPT_ERROR(BadValue);
			}
		}
		mpt_bacol_fini(bac);
		mpt_bacol_init(bac);
		bac->ivp = ivp;
		return len;
	}
	if (!strcasecmp(name, "atol")) {
		return mpt_vecpar_settol(&bac->atol, src, __MPT_IVP_ATOL);
	}
	if (!strcasecmp(name, "rtol")) {
		return mpt_vecpar_settol(&bac->rtol, src, __MPT_IVP_RTOL);
	}
	if (!strcasecmp(name, "nout") || !strcasecmp(name, "iout")) {
		int32_t oint = 10;
		
		if (src && (len = src->_vptr->conv(src, 'i', &oint)) < 0) return len;
		if (tolower(*name) != 'i') {
			--oint;
		}
		if (oint < 1) {
			return MPT_ERROR(BadValue);
		}
		bac->ivp.pint = oint;
		return len ? 1 : 0;
	}
	/* no interaction after prepare */
	if (bac->mflag.noinit >= 0) {
		return MPT_ERROR(BadOperation);
	}
	/* bacol parameters */
	if (!strcasecmp(name, "kcol")) {
		int32_t kcol = 2;
	
		if (src && (len = src->_vptr->conv(src, 'i', &kcol)) < 0) return len;
		else if (kcol < 2 || kcol > 10) return MPT_ERROR(BadValue);
		bac->kcol = kcol;
		return len ? 1 : 0;
	}
	if (!strcasecmp(name, "nint")) {
		int32_t nint = 10;
		
		/* initial grid value size fixed after init */
		if (bac->xy) {
			return MPT_ERROR(BadOperation);
		}
		if (src && (len = src->_vptr->conv(src, 'i', &nint)) < 0) return len;
		if (nint < 2 || nint > bac->nintmx) return MPT_ERROR(BadValue);
		bac->nint = nint;
		return len ? 1 : 0;
	}
	if (!strcasecmp(name, "nintmx")) {
		int32_t nintmx = 127;
		
		if (src && (len = src->_vptr->conv(src, 'i', &nintmx)) < 0) return len;
		if (nintmx < bac->nint) return MPT_ERROR(BadValue);
		bac->nintmx = nintmx;
		return len ? 1 : 0;
	}
	if (!strncasecmp(name, "dirichlet", 3)) {
		if (src && (len = src->_vptr->conv(src, 'i', &bac->mflag.bdir) < 0)) return len;
		if (!len) bac->mflag.bdir = 0;
		return len ? 1 : 0;
	}
	/* ode parameter */
	if (!strcasecmp(name, "stepinit") || !strcasecmp(name, "initstep")) {
		if (src && (len = src->_vptr->conv(src, 'd', &bac->initstep)) < 0) return len;
		bac->mflag.step = len ? 1 : 0;
		
		return len ? 1 : 0;
	}
	/* solving backend */
	if (!strcasecmp(name, "backend")) {
		return setBackend(bac, src);
	}
#ifdef MPT_BACOL_DASSL
	/* dassl parameter */
	if (bac->_backend == 'd' || bac->_backend == 'D') {
	if (!strcasecmp(name, "tstop")) {
		if (src && (len = src->_vptr->conv(src, 'd', &bac->bd.tstop)) < 0) return len;
		bac->mflag.tstop = len ? 1 : 0;
		return len ? 1 : 0;
	}
	/* dassl parameter */
	if (!strcasecmp(name, "maxstep")) {
		if (src && (len = src->_vptr->conv(src, 'i', &bac->mflag.mstep)) < 0) return len;
		if (!len) return bac->mflag.mstep = 0;
		return len ? 1 : 0;
	}
	if (!strcasecmp(name, "dasslbdf") || !strcasecmp(name, "bdf")) {
		if (src && (len = src->_vptr->conv(src, 'i', &bac->mflag.dbmax)) < 0) return len;
		if (!len) return bac->mflag.dbmax = 0;
		return len ? 1 : 0;
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
		return MPT_ENUM(TypeSolver);
	}
	if (!(name = prop->name)) {
		if ((pos = (intptr_t) prop->desc) < 0) {
			return MPT_ERROR(BadArgument);
		}
	}
	else if (!*name) {
		prop->name = "bacol"; prop->desc = "SPline PDE solver";
		prop->val.fmt = "ii"; prop->val.ptr = &bac->ivp;
		return MPT_SOLVER_ENUM(PDE);
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
		return mpt_vecpar_get(&bac->atol, &prop->val);
	}
	if (name ? !strcasecmp(name, "rtol") : pos == ++id) {
		prop->name = "rtol"; prop->desc = "relative tolerances";
		if (!bac) {
			prop->val.fmt = 0; prop->val.ptr = &bac->rtol;
			return id;
		}
		return mpt_vecpar_get(&bac->rtol, &prop->val);
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
